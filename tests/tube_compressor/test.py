import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("tube_compressor.net")
fs, vin = read_wav("vin.wav", scale=5)
fs, vout = read_wav("vout.wav", scale=250)
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("tube_compressor.net", "tube_compressor.h", dtype="double", instrument=True)
compile_run_cpp("tube_compressor")
cpp_vout = read_bin("output.bin")

skip = 0
vout_peak = np.max(np.abs(vout[skip:]))
rel_err = (cpp_vout[skip:] - vout[skip:]) / vout_peak
max_err = np.max(np.abs(rel_err))
print(f"Max Error: {max_err}")
assert max_err < 0.15

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    plt.plot(time, vin)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.grid()

    plt.figure()
    plt.plot(rel_err)
    plt.grid()

    plt.show()
