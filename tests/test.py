import subprocess
import numpy as np
import matplotlib.pyplot as plt
from PyLTSpice import RawRead

# Run LTspice
subprocess.run([
    r"/Applications/LTspice.app/Contents/MacOS/LTspice",
    "-b",
    "rc_lowpass.net"
])

# Read results
raw = RawRead("rc_lowpass.raw")
time = raw.get_trace("time").get_wave()
vin = raw.get_trace("V(vi)").get_wave()
vout = raw.get_trace("V(vo)").get_wave()

# Write vin to file
fs = 48000
# I shouldn't need to do this, but I can't seem to get ltspice to use a constant timestep!
time_uniform = np.arange(time[0], time[-1], 1/fs)
vin_uniform = np.interp(time_uniform, time, vin)
data_in = np.asarray(vin_uniform, dtype=np.float32)
with open("input.bin", "wb") as f:
    f.write(data_in.tobytes())

# generate data from C++
subprocess.run([
    "clang",
    "rc_lowpass.cpp",
    "--std=c++20",
    "-o",
    "rc_lowpass.exe"
])
subprocess.run(["./rc_lowpass.exe"])

# Compute error
cpp_data_out = np.fromfile("output.bin", dtype=np.float32)
vout_uniform = np.interp(time_uniform, time, vout)
error = (cpp_data_out - vout_uniform)
max_err = np.max(np.abs(error))
print(f"Max Error: {max_err}")

# Plot
plt.figure()
plt.plot(time, vin)
plt.plot(time, vout)
plt.plot(time_uniform, cpp_data_out, '--')
plt.grid()

plt.figure()
plt.plot(error)
plt.grid()

plt.show()
