import sys
import os
import subprocess
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("rc_cascade_stability.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav")
write_bin(vin, "input.bin")

# This fixture is 10 cascaded RC stages, each with a small (~1e-6) admittance --
# without sym_algebra.jai's automatic row scaling, det(A) for the resulting
# 10x10 system underflows float32 (see scratch/STABILITY_PLAN.md). Check
# codegen's stdout directly so a regression here fails loudly instead of just
# silently degrading accuracy.
codegen_result = subprocess.run(
    [codegen_exe, "rc_cascade_stability.net", "rc_cascade_stability.h"],
    capture_output=True, text=True)
print(codegen_result.stdout)
assert "Warning: det(A)" not in codegen_result.stdout, "det(A) underflow warning fired -- row scaling regressed"

# Generate C++ code
netlist_codegen("rc_cascade_stability.net", "rc_cascade_stability.h")
compile_run_cpp("rc_cascade_stability")
cpp_vout = read_bin("output.bin")

# Compute error
error = (cpp_vout - vout)
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")
assert max_err < 0.05

# Generate C code
netlist_codegen("rc_cascade_stability.net", "rc_cascade_stability_c.h", lang="c")
compile_run_c("rc_cascade_stability_c")
c_vout = read_bin("output.bin")

# Compute error (C)
error_c = (c_vout - vout)
max_err_c = np.max(np.abs(error_c))
print(f"Max Error (C): {max_err_c}")
assert max_err_c < 0.05

# Plot
if "plot" in sys.argv:
    plt.figure()
    time = np.arange(len(vin)) / fs
    plt.plot(time, vin)
    plt.plot(time, vout)
    plt.plot(time, cpp_vout, '--')
    plt.plot(time, c_vout, '--')
    plt.grid()

    # plt.figure()
    # plt.plot(error)
    # plt.grid()

    plt.show()