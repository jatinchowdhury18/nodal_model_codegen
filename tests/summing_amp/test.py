import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("summing_amp.net")
fs, vin1 = read_wav("vin1.wav")
fs, vin2 = read_wav("vin2.wav")
fs, vin3 = read_wav("vin3.wav")
fs, vout = read_wav("vout.wav")
write_bin(vin1, "input_v1.bin")
write_bin(vin2, "input_v2.bin")
write_bin(vin3, "input_v3.bin")

# Generate C++ code
netlist_codegen("summing_amp.net", "summing_amp.h")
compile_run_cpp("summing_amp")
cpp_vout = read_bin("output.bin")

# Compute error
error = (cpp_vout - vout)
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
assert max_err < 0.05

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin1)) / fs
    # plt.plot(time, vin1)
    # plt.plot(time, vin2)
    # plt.plot(time, vin3)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.grid()

    plt.figure()
    plt.plot(error)
    plt.grid()

    plt.show()
