#!/usr/bin/env python3
"""Single entry point: run the perf harness and generate a comparison
report in one go.

    python3 perf.py                          # all 5 cases, report vs. every existing tag
    python3 perf.py --case rc_lowpass         # just one case
    python3 perf.py --experiment reg-pressure-v1
    python3 perf.py --codegen-arg -opt_port_matrix     # tag gets the flag appended

Equivalent to running `run_perf.py` then `report.py --all` by hand --
this is just the "I want the whole thing done, don't make me remember two
commands" wrapper.
"""

import argparse
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)
from cases import CASE_NAMES


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--case", action="append", choices=CASE_NAMES, dest="cases",
                         help="run only this case (repeatable); default: all 5")
    parser.add_argument("--experiment", default=None,
                         help="experiment tag for perf/results/<tag>/; default: git short-hash (+'-dirty')")
    parser.add_argument("--codegen-arg", action="append", default=[], dest="codegen_args",
                         metavar="FLAG",
                         help="extra netlist_codegen flag applied to every case (repeatable)")
    args = parser.parse_args()

    run_cmd = [sys.executable, os.path.join(SCRIPT_DIR, "run_perf.py")]
    for name in (args.cases or []):
        run_cmd += ["--case", name]
    if args.experiment:
        run_cmd += ["--experiment", args.experiment]
    for flag in args.codegen_args:
        run_cmd += ["--codegen-arg", flag]

    result = subprocess.run(run_cmd, cwd=SCRIPT_DIR)
    if result.returncode != 0:
        sys.exit(result.returncode)

    result = subprocess.run([sys.executable, os.path.join(SCRIPT_DIR, "report.py"), "--all"], cwd=SCRIPT_DIR)
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
