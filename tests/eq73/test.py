import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from test_utils import *

os.chdir(os.path.dirname(os.path.abspath(__file__)))
cleanup()

# Run LTspice
run_spice("eq73.net")
fs, vin = read_wav("vin.wav")
fs, vout = read_wav("vout.wav", scale=2)
write_bin(vin, "input.bin")

# Generate C++ code. eq73.net has `split_here` markers (shelf/bell/hpf) so
# this is solved as 3 independent, well-conditioned pieces internally
# instead of one ill-conditioned 19-node system -- that's what makes float32
# viable here at all (it isn't, on the unsplit circuit; see STABILITY_PLAN.md).
netlist_codegen("eq73.net", "eq73.h", dtype="float")
compile_run_cpp("eq73")
cpp_vout = read_bin("output.bin")

# Compute error. First few samples skipped: LTspice's adaptive first
# transient step vs. our fixed-step model disagree right at the input
# edge by ~0.5 for a single sample, decaying to ~1e-3 by sample 8 -- a
# known startup-alignment artifact, unrelated to the circuit split.
error = (cpp_vout - vout)[8:]
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
