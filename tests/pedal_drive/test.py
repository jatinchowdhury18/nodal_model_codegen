import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("pedal_drive.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav", 1)
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("pedal_drive.net", "pedal_drive.h", dtype="double")
compile_run_cpp("pedal_drive")
cpp_vout = read_bin("output.bin")

# Our initialization is a little bit different from SPICE's
# so we skip a few samples at the beginning
skip = 100
error = (cpp_vout[skip:] - vout[skip:])
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
assert max_err < 0.02

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    plt.plot(time, vin)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.grid()

    plt.figure()
    plt.plot(cpp_vout[skip:] - vout[skip:])
    plt.grid()

    plt.show()
