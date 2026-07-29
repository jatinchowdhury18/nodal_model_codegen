import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("common_cathode.net")
fs, vin = read_wav("vin.wav", scale=5)
fs, vout = read_wav("vout.wav", scale=250)
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("common_cathode.net", "common_cathode.h", dtype="double")
compile_run_cpp("common_cathode")
cpp_vout = read_bin("output.bin")

# Our initialization is a little bit different from SPICE's
# so we skip a few samples at the beginning
skip = 0
error = (cpp_vout[skip:] - vout[skip:])
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
assert max_err < 1.5

# Generate C code
netlist_codegen("common_cathode.net", "common_cathode_c.h", lang="c", dtype="double")
compile_run_c("common_cathode_c")
c_vout = read_bin("output.bin")

error_c = (c_vout[skip:] - vout[skip:])
max_err_c = np.max(np.abs(error_c))
print(f"Max Error (C): {max_err_c}")
assert max_err_c < 1.5

# Plot
if "plot" in sys.argv:
    time = np.arange(len(vin)) / fs
    plt.figure()
    plt.plot(time, vin, label="vin")
    plt.plot(time, vout, label="vout (SPICE)")
    plt.plot(time, cpp_vout, '--', label="vout (C++)")
    plt.plot(time, c_vout, '--', label="vout (C)")
    plt.grid()
    plt.legend()

    vout_peak = np.max(np.abs(vout[skip:]))
    rel_err_cpp = 100.0 * (cpp_vout[skip:] - vout[skip:]) / vout_peak
    rel_err_c   = 100.0 * (c_vout[skip:]   - vout[skip:]) / vout_peak

    plt.figure()
    plt.plot(rel_err_cpp, label="C++ error")
    plt.plot(rel_err_c, label="C error")
    plt.ylabel("Relative error (% of vout peak)")
    plt.grid()
    plt.legend()

    plt.show()
