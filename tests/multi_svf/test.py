import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("multi_svf.net")
fs, vin = read_wav("vin.wav")
fs, vout_hp = read_wav("vout_hp.wav", scale=2)
fs, vout_bp = read_wav("vout_bp.wav")
fs, vout_lp = read_wav("vout_lp.wav")
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("multi_svf.net", "multi_svf.h")
compile_run_cpp("multi_svf")
cpp_vout_vh = read_bin("output_vh.bin")
cpp_vout_vb = read_bin("output_vb.bin")
cpp_vout_vl = read_bin("output_vl.bin")

# Compute error
error_vh = (cpp_vout_vh - vout_hp)
max_err = np.max(np.abs(error_vh))
print(f"Max Error (vh): {max_err}")
assert max_err < 0.05

error_vb = (cpp_vout_vb - vout_bp)
max_err = np.max(np.abs(error_vb))
print(f"Max Error (vb): {max_err}")
assert max_err < 0.05

error_vl = (cpp_vout_vl - vout_lp)
max_err = np.max(np.abs(error_vl))
print(f"Max Error (vl): {max_err}")
assert max_err < 0.05

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    # plt.plot(time, vin)
    plt.plot(time, vout_hp)
    plt.plot(time, vout_bp)
    plt.plot(time, vout_lp)
    plt.plot(time, cpp_vout_vh, '--')
    plt.plot(time, cpp_vout_vb, '--')
    plt.plot(time, cpp_vout_vl, '--')
    plt.grid()

    plt.figure()
    plt.plot(error_vh)
    plt.plot(error_vb)
    plt.plot(error_vl)
    plt.grid()

    plt.show()
