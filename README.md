# nodal_model_codegen

TODO:
- circuits with multiple inputs/outputs
- constant voltage sources
- nonlinear stuff
- non-ideal op-amp model?
- support Rust and/or Jai codegen?

`nodal_model_codegen` is a tool for generating optimized C++ or C audio DSP code
from [LTspice](https://www.analog.com/en/resources/design-tools-and-calculators/ltspice-simulator.html)
netlists, using [Modified Nodal Analysis (MNA)](https://en.wikipedia.org/wiki/Modified_nodal_analysis) and equivalent currents.

## What it does

Given an LTspice netlist (`.net` file) describing a passive or op-amp circuit,
`netlist_codegen` will:

1. Parse the netlist and identify all circuit elements
2. Apply circuit-level reductions (series and parallel element combinations)
3. Build and symbolically solve the MNA system of equations
4. Discretize reactive elements (capacitors, inductors) using trapezoidal integration
5. Apply code-level optimizations (common subexpression elimination, loop-invariant code motion)
6. Emit a self-contained C++ or C header file

## Usage

```
netlist_codegen <netlist_file> <header_file> [options]

Options:
  -lang      cpp (default) or c
  -type_name Numeric type for generated code (default: float)
  -namespace Wrap generated code in a C++ namespace (C++ only)
```

For example, to generate a C++ header from a netlist:

```
netlist_codegen my_circuit.net my_circuit.h
```

To generate a C header wrapped in a custom namespace:

```
netlist_codegen my_circuit.net my_circuit.h -lang cpp -namespace MyCircuit
```

## Example

Given the following netlist for a first-order RC low-pass filter:

```
* RC Lowpass Filter
V1 vi 0 PULSE(-1 1 0 20m 1u 1u 20m)
R1 vo vi 1k
C1 vo 0 1u IC=0
```

`netlist_codegen` will produce the following C++ header:

```cpp
#pragma once

struct Params {
    float R1 = 1.0e+03f;
    float C1 = 1.0e-06f;
};

struct State {
    float zC1 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    const auto gR1 = 1.0f / params.R1;
    const auto gC1 = 2.0f * sample_rate * params.C1;

    const auto _t0 = (1 / (gR1 + gC1));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vo = (((gR1 * vi) + zC1) * _t0);
            const auto tC1 = (gC1 * (vo - 0));

            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
    }
}
```

The generated `compute` function takes multi-channel input and output buffers in
the same style as many audio plugin APIs (e.g. JUCE, CLAP). The `Params` struct
holds component values (with defaults populated directly from the netlist), and
the `State` struct holds the per-channel state for all reactive elements.

## A few other thoughts

### Circuit reductions

Before solving the MNA system, `netlist_codegen` attempts to reduce the circuit
by combining elements that share an internal node (a node that is not an input,
output, or ground). The following reductions are supported:

- **Series R+R, C+C** → a single equivalent element
- **Parallel R‖R, C‖C** → a single equivalent element
- **Series R-C** → a single compact RC element with an efficient trapezoidal integration formulation
- **Parallel R‖C** → similarly combined into a single efficient element

These reductions shrink the MNA matrix that needs to be solved, and can
meaningfully reduce the number of arithmetic operations in the generated code.

### Code-generation optimizations

After solving the MNA equations symbolically, three passes are run over the
resulting expressions before any code is emitted:

- **Common subexpression elimination (CSE)** — shared sub-expressions are
  extracted into named temporaries (`_t0`, `_t1`, …) to avoid recomputing them
- **Loop-invariant code motion (LICM)** — temporaries that do not depend on the
  per-sample input are hoisted outside the sample loop
- **Reciprocal hoisting** — when a loop-invariant temporary appears only as a
  divisor inside the sample loop, it is replaced by its reciprocal, turning
  an inner-loop division into a cheaper multiply

### Building the tool

`netlist_codegen` is written in Jai. To build it you will need a Jai compiler.
With the compiler on your `PATH`, run:

```
jai src/netlist_codegen.jai
```

This produces the `netlist_codegen` binary in `src/`.

### Running the tests

The test suite validates each generated model against a reference LTspice
simulation. You will need [LTspice](https://www.analog.com/en/resources/design-tools-and-calculators/ltspice-simulator.html),
Python 3, and the following Python packages installed:

```
pip install numpy scipy matplotlib
```

Then run:

```
cd tests
bash run_tests.sh
```

Currently tested circuits:

- `rc_lowpass` — 1st-order RC low-pass filter
- `res_highpass` — Resonant RC high-pass filter
- `sk_lpf` — 2nd-order Sallen-Key low-pass filter with ideal op-amp
- `reductions` — exercises all series/parallel element-reduction paths
- `eq_filter` — multi-op-amp EQ filter

Each test runs LTspice on the original netlist to produce a reference output,
then generates and compiles the C++ model, runs it on the same input, and
asserts that the maximum absolute error is below a small threshold.

## License

`nodal_model_codegen` is open source, and is licensed under the
MIT license.

Enjoy!
