import subprocess
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(__file__)))
from netlist_codegen import netlist_codegen

def cleanup():
    os.system("rm -f *.bin")
    os.system("rm -f *.raw")
    os.system("rm -f *.wav")
    os.system("rm -f *.log")
    os.system("rm -f *.exe")

def run_spice(netlist_file):
    subprocess.run([
        r"/Applications/LTspice.app/Contents/MacOS/LTspice",
        "-b",
        netlist_file
    ])

def read_wav(wav_file):
    fs, v = wavfile.read(wav_file)
    v = v.astype(np.float32) / 2147483648.0
    return fs, v

def write_bin(x, bin_file):
    with open(bin_file, "wb") as f:
        f.write(x.astype(np.float32).tobytes())

def read_bin(bin_file):
    return np.fromfile(bin_file, dtype=np.float32)

def compile_run_cpp(name):
    subprocess.run([
        "clang",
        f"{name}.cpp",
        "--std=c++20",
        "-o",
        f"{name}.exe"
    ])
    subprocess.run([f"./{name}.exe"])
