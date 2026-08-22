#!/usr/bin/env python3
"""Runs the perf harness (no report generation -- see perf.py for the
one-shot "run everything and report" entry point).

Two-stage orchestration:
  1. (this script, Python) run netlist_codegen for each selected case,
     parsing its stdout for codegen-time stats (FLOPs, temp count) that
     only the codegen CLI knows about, then compile run_perf.cpp.
  2. (run_perf.exe, C++) do the actual timed runs + HW counter reads,
     writing one JSON file per case with runtime stats.

This script then merges its own codegen-time stats into those same JSON
files (git commit, timestamp, FLOPs, temp count) so each
perf/results/<experiment_tag>/<case>.json ends up self-contained.
"""

import argparse
import datetime
import json
import os
import platform
import re
import subprocess
import sys
import time

SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
TESTS_DIR   = os.path.join(PROJECT_DIR, "tests")
CODEGEN_EXE = os.path.join(PROJECT_DIR, "src", "netlist_codegen")
GENERATED_DIR = os.path.join(SCRIPT_DIR, "generated")

sys.path.insert(0, SCRIPT_DIR)
from cases import CASES, CASE_NAMES

FLOP_RE = re.compile(r"Outer loop: (?P<outer>.+)\r?\n\s*Inner loop: (?P<inner>.+)")
TEMP_RE = re.compile(r"Temp vars: (?P<outer>\d+) outer, (?P<inner>\d+) inner, (?P<total>\d+) total")
OPS_RE  = re.compile(r"Estimated worst-case ops/sample: (?P<total>\d+)")


def git(*args):
    result = subprocess.run(["git", *args], cwd=PROJECT_DIR, capture_output=True, text=True)
    return result.stdout.strip(), result.returncode


def experiment_tag_default():
    commit, rc = git("rev-parse", "--short", "HEAD")
    if rc != 0:
        return "unknown"
    _, dirty_rc = git("diff", "--quiet")
    return commit if dirty_rc == 0 else f"{commit}-dirty"


def generate_header(case):
    net_path = os.path.join(TESTS_DIR, case["net"])
    header_path = os.path.join(GENERATED_DIR, f"{case['name']}.h")
    cmd = [CODEGEN_EXE, net_path, header_path,
           "-type_name", case["dtype"], "-namespace", case["name"]]
    cmd += ["-cpu", "m1"] # @TODO: make this a parameter
    # Per-case extra codegen flags (see cases.py), so flag-gated variants are
    # measured reproducibly instead of by hand-generating into perf/generated/.
    cmd += case.get("extra_args", [])

    start = time.perf_counter()
    result = subprocess.run(cmd, capture_output=True, text=True)
    codegen_ms = 1000 * (time.perf_counter() - start)

    if result.returncode != 0:
        print(f"codegen failed for {case['name']}:\n{result.stdout}\n{result.stderr}", file=sys.stderr)
        sys.exit(1)

    # A circuit split into sub-circuits (see eq73) logs one of each of these
    # blocks per sub-circuit, so sum across all of them: this way multi-unit
    # circuits get whole-circuit totals instead of just the first sub-circuit's.
    output = result.stdout + result.stderr
    flop_ms = list(FLOP_RE.finditer(output))
    temp_ms = list(TEMP_RE.finditer(output))
    ops_ms  = list(OPS_RE.finditer(output))

    return {
        "codegen_ms": codegen_ms,
        "outer_ops": "; ".join(m.group("outer") for m in flop_ms) if flop_ms else None,
        "inner_ops": "; ".join(m.group("inner") for m in flop_ms) if flop_ms else None,
        "temp_count_outer": sum(int(m.group("outer")) for m in temp_ms) if temp_ms else None,
        "temp_count_inner": sum(int(m.group("inner")) for m in temp_ms) if temp_ms else None,
        "temp_count_total": sum(int(m.group("total")) for m in temp_ms) if temp_ms else None,
        "estimated_worst_case_ops_per_sample": sum(int(m.group("total")) for m in ops_ms) if ops_ms else None,
    }


def compile_driver():
    driver_exe = os.path.join(SCRIPT_DIR, "run_perf.exe")
    cmd = [
        "clang++", os.path.join(SCRIPT_DIR, "run_perf.cpp"),
        "--std=c++20", "-O3", "-I", SCRIPT_DIR,
        "-D_CRT_SECURE_NO_WARNINGS",
        "-o", driver_exe,
    ]
    if platform.system() == "Linux":
        cmd.append("-ldl")
    print("Compiling perf driver: " + " ".join(cmd))
    result = subprocess.run(cmd)
    if result.returncode != 0:
        print("Failed to compile run_perf.cpp", file=sys.stderr)
        sys.exit(1)
    return driver_exe


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--case", action="append", choices=CASE_NAMES, dest="cases",
                         help="run only this case (repeatable); default: all cases")
    parser.add_argument("--codegen-arg", action="append", default=[], dest="codegen_args",
                         metavar="FLAG",
                         help="extra netlist_codegen flag applied to every case (repeatable), "
                              "e.g. --codegen-arg -opt_port_matrix")
    parser.add_argument("--experiment", default=None,
                         help="experiment tag for perf/results/<tag>/; default: git short-hash (+'-dirty')")
    args = parser.parse_args()

    selected = args.cases if args.cases else CASE_NAMES
    # Fold the codegen flags into the default tag. Without this a flagged run
    # silently overwrites the same commit's unflagged results directory.
    tag = args.experiment
    if not tag:
        tag = experiment_tag_default()
        for flag in args.codegen_args:
            tag += "-" + flag.lstrip("-")

    os.makedirs(GENERATED_DIR, exist_ok=True)
    out_dir = os.path.join(SCRIPT_DIR, "results", tag)
    os.makedirs(out_dir, exist_ok=True)

    print(f"Experiment tag: {tag}")

    # Headers get generated for all cases regardless of selection, since
    # run_perf.cpp #includes all of them unconditionally (avoids conditional
    # compilation for a fixed, small case list) -- only the *timing* is
    # subset-selectable via run_perf.exe's --case flag.
    # Global flags append to whatever per-case extra_args a case already carries.
    active = CASES
    if args.codegen_args:
        active = [dict(c, extra_args=list(c.get("extra_args", [])) + args.codegen_args) for c in CASES]

    codegen_stats = {}
    for case in active:
        print(f"Generating {case['name']}...")
        codegen_stats[case["name"]] = generate_header(case)

    driver_exe = compile_driver()

    cmd = [driver_exe, "--out-dir", out_dir]
    for name in selected:
        cmd += ["--case", name]
    print("Running: " + " ".join(cmd))
    result = subprocess.run(cmd, cwd=SCRIPT_DIR)
    if result.returncode != 0:
        print("run_perf.exe failed", file=sys.stderr)
        sys.exit(1)

    commit, _ = git("rev-parse", "--short", "HEAD")
    _, dirty_rc = git("diff", "--quiet")
    timestamp = datetime.datetime.now(datetime.timezone.utc).isoformat()
    host = platform.platform()

    for name in selected:
        json_path = os.path.join(out_dir, f"{name}.json")
        with open(json_path) as f:
            data = json.load(f)
        data["experiment_tag"] = tag
        data["git_commit"] = commit
        data["git_dirty"] = dirty_rc != 0
        data["timestamp"] = timestamp
        data["host"] = host
        data["codegen"] = codegen_stats[name]
        data["codegen_args"] = args.codegen_args
        with open(json_path, "w") as f:
            json.dump(data, f, indent=2)

    print(f"\nResults written to {out_dir}/")


if __name__ == "__main__":
    main()
