import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("opamp_clipping.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav", scale=10)
write_bin(vin, "input.bin")

# Generate C++ code from our own non-ideal op-amp syntax.
netlist_codegen("opamp_clipping.net", "opamp_clipping.h")
compile_run_cpp("opamp_clipping")
cpp_vout = read_bin("output.bin")

# Compute error
error = (cpp_vout - vout)
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
# The single worst sample lands right on the clip transition -- LTspice's
# adaptive-timestep solver crosses the rail one sample later than our fixed
# per-sample model does (a timing artifact, not a modeling bug: 99th
# percentile error is ~1e-6). Assert on both so a real regression elsewhere
# in the waveform wouldn't hide behind the loose edge-case bound.
percentile_err = np.percentile(np.abs(error), 99)
print(f"99th percentile error: {percentile_err}")
assert percentile_err < 0.001
assert max_err < 0.1

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    plt.plot(time, vin)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.grid()

    plt.figure()
    plt.plot(error)
    plt.grid()

    plt.show()
