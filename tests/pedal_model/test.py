import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("pedal_model.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav")
write_bin(vin, "input.bin")

# Generate C++ code
netlist_codegen("pedal_model.net", "pedal_model.h", dtype="double")
compile_run_cpp("pedal_model")
cpp_vout = read_bin("output.bin")

error = (cpp_vout - vout)
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
assert max_err < 0.02

# Generate C code
netlist_codegen("pedal_model.net", "pedal_model_c.h", lang="c", dtype="double")
compile_run_c("pedal_model_c")
c_vout = read_bin("output.bin")

error_c = (c_vout - vout)
max_err_c = np.max(np.abs(error_c))
print(f"Max Error (C): {max_err_c}")
assert max_err_c < 0.02

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    plt.plot(time, vin)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.plot(time, c_vout, '--')
    plt.grid()

    plt.figure()
    plt.plot(cpp_vout - vout)
    plt.plot(c_vout - vout)
    plt.grid()

    plt.show()
