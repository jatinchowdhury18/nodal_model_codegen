import subprocess
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

import platform
import sys
import os
_project_root = os.path.dirname(os.path.dirname(__file__))
sys.path.append(_project_root)
sys.path.append(os.path.join(_project_root, "legacy"))
# from netlist_codegen import netlist_codegen

system = platform.system()
if system == "Windows":
    ltspice_exe = os.path.expanduser("~") + "/AppData/Local/Programs/ADI/LTspice/LTspice.exe"
    codegen_exe = "../../src/netlist_codegen.exe"
elif system == "Darwin":
    ltspice_exe = r"/Applications/LTspice.app/Contents/MacOS/LTspice"
    codegen_exe = "../../src/netlist_codegen"

def cleanup():
    os.system("rm -f *.bin")
    os.system("rm -f *.raw")
    os.system("rm -f *.wav")
    os.system("rm -f *.log")
    os.system("rm -f *.exe")

def run_spice(netlist_file):
    subprocess.run([
        ltspice_exe,
        "-b",
        netlist_file
    ])

def read_wav(wav_file, scale = 1):
    fs, v = wavfile.read(wav_file)
    v = scale * v.astype(np.float32) / 2147483648.0
    return fs, v

def write_bin(x, bin_file):
    with open(bin_file, "wb") as f:
        f.write(x.astype(np.float32).tobytes())

def read_bin(bin_file):
    return np.fromfile(bin_file, dtype=np.float32)

def netlist_codegen(netlist_file, header_file, lang="cpp", namespace=None):
    cmd = [
        codegen_exe,
        netlist_file,
        header_file
    ]
    if lang != "cpp":
        cmd += ["-lang", lang]
    if namespace is not None:
        cmd += ["-namespace", namespace]
    subprocess.run(cmd)

def compile_run_cpp(name):
    subprocess.run([
        "clang",
        f"{name}.cpp",
        "--std=c++20",
        "-D_CRT_SECURE_NO_WARNINGS",
        # "-g",
        "-o",
        f"{name}.exe"
    ])
    subprocess.run([f"./{name}.exe"])

def compile_run_c(name):
    subprocess.run([
        "clang",
        f"{name}.c",
        "--std=c11",
        "-D_CRT_SECURE_NO_WARNINGS",
        # "-g",
        "-o",
        f"{name}.exe"
    ])
    subprocess.run([f"./{name}.exe"])
