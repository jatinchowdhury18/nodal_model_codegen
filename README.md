# nodal_model_codegen

`nodal_model_codegen` is a tool for generating optimized C or C++ audio DSP code
from [LTspice](https://www.analog.com/en/resources/design-tools-and-calculators/ltspice-simulator.html)
netlists, using [Modified Nodal Analysis (MNA)](https://en.wikipedia.org/wiki/Modified_nodal_analysis) and "equivalent currents".

_This work is currently intended for research purposes only.
There are plans to publish an executable "release build"
after the system has stabilized and undergone further
testing._

## What it does

Given an LTspice netlist (`.net` file) describing a circuit, `netlist_codegen` will:

1. Parse the netlist and identify all circuit elements
2. Apply circuit-level reductions (series and parallel element combinations)
3. Build and symbolically solve the MNA system of equations
4. Discretize reactive elements (capacitors, inductors) using trapezoidal integration and "equivalent currents"
5. Derive iterative solutions for resolving nonlinear current/voltage relationships
6. Apply code-level optimizations (common subexpression elimination, loop-invariant code motion, etc)
7. Emit a self-contained C++ or C header file

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

To generate a C header wrapped with double-precision computations:

```
netlist_codegen my_circuit.net my_circuit.h -lang cpp -type_name double
```

## Example

Given the following netlist for a first-order RC low-pass filter:

```
* RC Lowpass Filter
V1 vi 0 PULSE(-1 1 0 20m 1u 1u 20m)
R1 vo vi 1k
C1 vo 0 1u IC=0
```

`netlist_codegen` might produce the following
C++ header (with comments added):

```cpp
#pragma once

struct Params {
    float R1 = 1.0e+03f; // R1 resistance (Ohms)
    float C1 = 1.0e-06f; // C1 capacitance (Farads)
};

struct State {
    float zC1 {}; // C1 "equivalent current" state
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    const auto gR1 = 1.0f / params.R1; // R1 conductance
    const auto gC1 = 2.0f * sample_rate * params.C1; // C1 admittance

    const auto _t0 = (1 / (gR1 + gC1)); // temporary "coefficient"
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n]; // input voltage

            const auto vo = (((gR1 * vi) + zC1) * _t0); // output voltage
            const auto tC1 = (gC1 * (vo - 0));

            zC1 = 2 * tC1 - zC1; // C1 state update

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
    }
}
```

The generated `compute` function takes multi-channel input and output buffers in
the same style as many audio plugin APIs (e.g. JUCE, CLAP, etc). The `Params` struct
holds component values (with defaults populated directly from the netlist), and
the `State` struct holds the per-channel state for all reactive elements.

## Netlist Directives & Conventions

`netlist_codegen` reads standard LTspice `.net` netlists, but recognizes a
handful of extra conventions on top of plain SPICE syntax, in order to give
the user more control over the generated code. All additional directives are
either ordinary `*` comment lines or `;` trailing comments on an element
line, so they're invisible to LTspice itself.

### Inputs and Outputs

Inputs are picked up **implicitly**: any non-`fixed`, non-`DC` voltage
source (e.g. `V1 vi 0 PULSE(...)` or `SIN(...)`) is treated as a per-sample
input, named after its positive node. If a voltage source is specified as "DC"
or "fixed", the source will instead be treated as a **constant** and folded
into the `Params` struct.

```
VCC vp 0 DC 250      ; constant, becomes a Params field
Vbias vb 0 4.5 fixed ; constant, same as above
V1   vi 0 PULSE(...) ; per-sample input
```

By default, any node labelled "vo" will be treated as an output.
Alternatively, outputs can be explicitly defined via a `* output:<node>`
comment line. Multiple outputs are supported by repeating the comment:

```
* output:vh
* output:vb
* output:vl
```

### `no_combine`

Adding `no_combine` anywhere on an element's line (as a comment) excludes
that element from series/parallel reduction, e.g. when you need to keep a
node visible for probing or a downstream `nodal_model:` reference:

```
R1 vo vi 1k ; no_combine
```

### `netlist_skip`

Adding `netlist_skip` anywhere on a line drops it from codegen entirely,
while LTspice still simulates it. This pattern is used in the test
circuits to scale the output signals before `.wav` export.

```
Escale1 vi_scaled 0 value = {V(vi)*0.2} ; netlist_skip
```

### `iter_solve(...)`

A `* iter_solve(method=<name>, max_iter=<int>, tol=<val>, alpha=<val>)`
comment line selects which iterative method resolves nonlinear elements in
that netlist, and overrides its default iteration count and squared-error
tolerance. The default configuration is:

```
* iter_solve(method=newton, max_iter=20, tol=1.0e-5)
```

Supported `method` values:

| Method | Notes |
|---|---|
| `newton` (default) | Newton-Raphson. The Jacobian is derived symbolically at codegen time and the linear system `J*delta = -f` is solved in closed form, so each iteration is a single direct step. Fastest per-iteration and the best-tested path; the DC operating-point solve in `reset()` always uses this, regardless of the netlist's `iter_solve` setting. |
| `fixed_point` | Damped Picard iteration: `x += alpha*(closed_form(x) - x)`. No Jacobian at all. `alpha` (default `0.5`) is the damping factor -- lower is more stable but converges more slowly; `alpha=1.0` is plain (undamped) fixed-point iteration. |
| `broyden` | Quasi-Newton: maintains a running inverse-Jacobian estimate, seeded from one real (symbolic) Newton step and then updated every iteration via a Sherman-Morrison rank-1 (Broyden) update plus a backtracking line search, entirely at runtime. For a single-unknown cluster this is exactly the secant method. |

**`fixed_point` and `broyden` only run for single-unknown clusters** (one
device, e.g. a lone diode/JFET/triode not sharing a node with another
nonlinear device). For a multi-device cluster -- a BJT's coupled vBC/vBE, a
triode's vPK/vGK, two diodes sharing a node -- they silently fall back to
`newton` for that cluster (logged as a warning during codegen). This isn't
a hard theoretical limit, just where testing landed: verified against the
test suite, both converge identically to Newton on a single unknown, but
on a multi-unknown stiff/exponential cluster, the true Jacobian's entries
can differ by many orders of magnitude (e.g. a diode's `Is ~ 1e-14`), and
neither an identity-seeded Broyden update nor damped Picard iteration
tracked that curvature well enough to reliably converge -- Broyden was
observed wandering between several non-converged residual plateaus rather
than diverging outright, even with the line search in place. Making either
one robust there would need real preconditioning or a trust-region
strategy, which is out of scope for now; `newton` (or restructuring the
circuit so the devices don't end up in one cluster, if that's an option)
is the reliable choice for those.

`fixed_point` and `broyden` also don't implement the active-set
rail-clamping Newton uses for a clipping op-amp's output stage
(`nodal_model:` with `Vsat+`/`Vsat-`); they fall back to a plain post-step
clamp for that unknown instead.

@TODO

Some of the internal naming needs changes (e.g. `generate_newton_constants()` should be something like `generate_iter_solve_constants()`). Same with the variable names in the generated code (e.g. `newton_tol_sq`).

I think all the solvers should have a "damping factor", but probably have it default to 1 for Newton and Broyden?

I don't like that we're still generating all the Newton vars, even without using them [@codegen.jai (776:780)]. What are some ways around it?

### `sub-circuit`

Modelling a large circuit with nodal analysis can be
difficult, since the solved equations often become
poorly conditioned (especially with single-precision
`float`s), and may have poor performance.
`* sub-circuit: <name> <in_node> <out_node>` allows
a circuit to be broken up into "sub-circuits" that
are each modelled as their own independent systems:

```
* sub-circuit: stage1  vi vo1
* sub-circuit: stage2 vo1 vo2
* sub-circuit: stage3 vo2  vo
```

For each declared piece, `netlist_codegen` emits its own
`Params`/`State`/`compute()`/etc. with `_name` appended to
each name, as well as the usual ones for the full circuit.

### Nonlinear Elements

`netlist_codegen` handles standard SPICE nonlinear elements, including
Diodes, BJTs, and JFETs. These use ordinary SPICE element/model syntax,
no special tagging needed:

```
.model D1N914 D (IS=2.52E-9 N=1.752)
D1 vo 0 D1N914

.model Q2N5089 NPN (IS=5E-14 BF=600 BR=50)
Q1 vc vb ve Q2N5089

.model 2N5485 NJF (BETA=2m VTO=-1)
J1 vd vg vs 2N5485
```

### Vacuum tubes: `.model ... TRIODE`

`TRIODE` is a `netlist_codegen`-only device kind (not supported by SPICE),
using the Dempwolf & Zolzer (DAFx-11) model. An `X<name> <plate> <grid>
<cathode> <model>` line instantiates one:

```
.model 12AX7 TRIODE (GK=2.242e-3 MU=103.2 GAMMA=1.26 CK=3.40 GG=6.177e-4 XI=1.314 CG=9.901 IG0=8.025e-8)
X1 vpl vg vk 12AX7
```

In the test circuits, netlists also carry an inline `.SUBCKT`/`.ENDS`
block (with the same name) implementing the same model as a reference.
`netlist_codegen` parses the `.model` card and skips the `.SUBCKT` body
entirely.

### Non-ideal op-amps: `.model ... OPAMP` and `nodal_model:`

SPICE has no native op-amp primitive - an ideal op-amp is just a VCVS (denoted by the `E` prefix), and by default `netlist_codegen` will treat an `E` line as an ideal
(infinite gain, infinite bandwidth) op-amp.

In order to opt-in to non-ideal behaviour, define a
`.model <name> OPAMP (...)` card and tag the instance line
with a trailing `; nodal_model: <name>` comment:

```
.model FINITEGAIN OPAMP (Aol=300k)
Eop n7 0 v5 v6 300k ; nodal_model: FINITEGAIN
```

All `OPAMP` parameters are optional. If a parameter is not set, the model will fall back to the "ideal" behaviour.

| Param | Meaning |
|---|---|
| `Aol` | Open-loop gain |
| `GBW` / `Ccomp` | Gain-bandwidth product, or an explicit compensation cap (takes precedence over `GBW` if both are set) |
| `Rin` / `Rout` | Input / output impedance |
| `Vsat+` / `Vsat-` | Output clipping rails |
| `slew` | Slew-rate limit |
| `Vos` / `Ios` / `Ibias` | Input offset voltage / offset current / bias current |

The `nodal_model:` tag works on both `E` and `X` lines (see e.g.
`rat_full.net`). By default, the op-amp's out/+/− nodes are read
positionally from that line's tokens (position 1/3/4, matching an `E` ideal
VCVS); `out=`/`pos=`/`neg=` overrides are needed when the tagged line's
layout doesn't match that (e.g. an `X` line with a different pin order):

```
.model LM308 OPAMP (Aol=200k GBW=50Meg Rin=1Meg Rout=50 Vsat+=9 Vsat-=0 Vos=2m Ios=2n Ibias=3n)
XU2 vpos vneg vpp 0 v2 NCA NCB LM308 ; nodal_model: LM308 out=v2 pos=vpos neg=vneg
```

### Part Name Identifiers

A `.model` name that isn't a valid identifier (e.g. a real part number like
`2N5485` or `12AX7`) gets a `_` prefix when it appears in the generated
`Params` struct (`_2N5485_Beta`, `_12AX7_Gk`, etc.) but netlist lookups
still use the raw, un-prefixed name.

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

### Nonlinear Circuit Elements

Nonlinear elements are solved with an iterative solver (Newton-Raphson by
default -- see [`iter_solve(...)`](#iter_solve) above), layered on top of
the same symbolic MNA core used for the linear elements: `netlist_codegen`
leaves each device's branch current as a free symbol, so the linear solve
still gives closed-form node voltages parametrized by those currents. From
there, the device's real I-V law is substituted in as an expression. The
following device types are currently supported:

- **Diode / antiparallel diode pair**: Shockley exponential I-V law
- **BJT**: Ebers-Moll, with independent base-collector/base-emitter unknowns
- **JFET**: square-law I-V, clamped at pinch-off
- **TRIODE**: Dempwolff and Zolzer model 
- **Non-ideal op-amp output stage**: Vsat clipping and slew-rate limiting,
  as a hard clamp on the same free-symbol unknown used for the linear VCVS

Devices whose branch currents depend on each other's junction voltages (e.g.
a BJT's vBC/vBE, or two diodes sharing a node) are grouped into "clusters"
and solved jointly, one iterative loop per cluster. Each unknown also gets a
per-iteration step limiter, so a bad step doesn't wander into a region the I-V
law approximates badly. The solver method, tolerance, and iteration count are
tunable per netlist via an `iter_solve(method=.. max_iter=.. tol=..)` comment
directive.

### Code-generation optimizations

After solving the MNA equations symbolically, three passes are run over the
resulting expressions before any code is emitted:

- **Common subexpression elimination (CSE)**: shared sub-expressions are
  extracted into named temporaries (`_t0`, `_t1`, …) to avoid recomputing them
- **Loop-invariant code motion (LICM)**: temporaries that do not depend on the
  per-sample input are hoisted outside the sample loop
- **Reciprocal hoisting**: when a loop-invariant temporary appears only as a
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

The tests currently contain a number of "real-world" audio circuits,
as well as more specific "test cases" to make that certain parts of the
system are behaving as expected. Each test runs LTspice on the original
netlist to produce a reference output, then generates and compiles the
C/C++ model, runs it on the same input, and asserts that the maximum absolute
error is below a small threshold. The tests will optionally generate
plots of the signals and error.

Note: sometimes the error plots will show "spikes" in the error,
typically when the signal is rising or falling very quickly.
This appears to be due to SPICE's adaptive time-step behaviour
rather than a true inaccuracy in the simulation.

## License

`nodal_model_codegen` is open source, and is licensed under the
MIT license.

Enjoy!
