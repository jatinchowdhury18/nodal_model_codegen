import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("dc_coupled_x2.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav", 10)
write_bin(vin, "input.bin")

netlist_codegen("dc_coupled_x2.net", "dc_coupled_x2.h", dtype="float")
compile_run_cpp("dc_coupled_x2")
cpp_vout = read_bin("output.bin")

error = (cpp_vout - vout)
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
    plt.plot(cpp_vout - vout)
    plt.grid()

    plt.show()
