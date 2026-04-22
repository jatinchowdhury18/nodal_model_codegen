import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("eq_filter2.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav", scale=2)
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("eq_filter2.net", "eq_filter2.h", dtype="double")
compile_run_cpp("eq_filter2")
cpp_vout = read_bin("output.bin")

# Compute error
error = (cpp_vout - vout)[5:]
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
assert max_err < 0.05

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
