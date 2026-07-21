import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("opamp_complete.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav", scale=20)
write_bin(vin, "input.bin")

# Generate C++ code from our own non-ideal op-amp syntax.
netlist_codegen("opamp_complete.net", "opamp_complete.h")
compile_run_cpp("opamp_complete")
cpp_vout = read_bin("output.bin")

# Compute error
# Using 99th percentile since LTSpice sub-sample timing errors
# cause error spikes on transients
error = (cpp_vout - vout)
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
percentile_err = np.percentile(np.abs(error), 99)
print(f"99th percentile error: {percentile_err}")
assert percentile_err < 0.001
assert max_err < 10.0

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
