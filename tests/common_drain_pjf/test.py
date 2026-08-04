import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("common_drain_pjf.net")
fs, vin = read_wav("vin.wav", scale=10)
fs, vout = read_wav("vout.wav", scale=2)
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("common_drain_pjf.net", "common_drain_pjf.h", dtype="double")
compile_run_cpp("common_drain_pjf")
cpp_vout = read_bin("output.bin")

skip = 0
error = (cpp_vout[skip:] - vout[skip:])
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
# assert max_err < 0.02

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    plt.plot(time, vin - 4.5)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.grid()

    plt.figure()
    plt.plot(cpp_vout[skip:] - vout[skip:])
    plt.grid()

    plt.show()
