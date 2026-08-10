#!/usr/bin/env python3
"""Best-effort static analysis of a case's compute() via llvm-mca.

Standalone/optional -- not wired into run_perf.py's default flow. Run it
by hand once llvm-mca is confirmed working in your terminal:

    python3 run_llvm_mca.py <case_name> [--mcpu apple-m1] [--summary-only]

llvm-mca isn't part of Apple's default clang toolchain -- install via
`brew install llvm` and put it on PATH first:
    export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

What this buys over the dynamic HW-counter numbers run_perf.cpp already
collects: llvm-mca is deterministic (no OS jitter/branch-predictor warmup/
thermal throttling) and needs no root/PMU access, so it's useful for
isolating "did this codegen change help" from run-to-run measurement noise.
Its per-instruction resource-pressure view is also a reasonable static
proxy for register pressure, since spilled temporaries show up as ordinary
load/store instructions in the post-regalloc assembly it analyzes.

NOTE: a standalone hand-written .s file (no real function, just a loop)
made llvm-mca hang indefinitely while developing this -- even with an
explicit `-mcpu` (ruling out `-mcpu=native` CPU-detection as the cause) and
stdin redirected from /dev/null. Root cause not fully identified, but it
went away once testing switched to real compiler-generated assembly (this
script's actual approach) -- worked cleanly on both `rc_lowpass` and
`pedal_drive`. Kept the hard subprocess timeout below anyway, since a
static-analysis step silently hanging the whole perf run would be a bad
failure mode either way.
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
GENERATED_DIR = os.path.join(SCRIPT_DIR, "generated")

MCA_TIMEOUT_SEC = 30


def extract_compute_asm(asm_text, case_name):
    """Pull just the mangled `compute` function's instructions out of a full
    .s file, from its global label down to `.cfi_endproc`. Falls back to the
    whole file (with a warning) if that pattern isn't found -- assembly
    output shape varies enough across compiler versions that this is worth
    keeping permissive rather than brittle.
    """
    # Mangled C++ symbol for `<case_name>::compute` contains "7computeE".
    label_re = re.compile(r"^(_?_ZN\d*" + re.escape(case_name) + r".*7computeE\S*):", re.MULTILINE)
    m = label_re.search(asm_text)
    if not m:
        print(f"warning: couldn't find a 'compute' label for {case_name} in the assembly "
              f"(compiler/mangling may not match what this regex expects) -- "
              f"handing llvm-mca the whole file instead", file=sys.stderr)
        return asm_text

    start = m.start()
    end_m = re.search(r"\.cfi_endproc", asm_text[start:])
    end = start + end_m.end() if end_m else len(asm_text)
    return asm_text[start:end]


def run_for_case(case_name, mcpu):
    header_path = os.path.join(GENERATED_DIR, f"{case_name}.h")
    if not os.path.exists(header_path):
        print(f"{header_path} doesn't exist -- run run_perf.py first to generate it", file=sys.stderr)
        sys.exit(1)

    with tempfile.TemporaryDirectory() as tmp:
        # generated/<case>.h's `compute()` has internal (static) linkage and
        # nothing in this stub calls it -- at -O3 an unused static function
        # gets dead-code-eliminated entirely and there's no symbol left for
        # llvm-mca to analyze. Taking its address into a non-static global
        # forces the compiler to keep a real, out-of-line definition (a
        # relocation needs somewhere to point) without actually calling it
        # (which would risk it getting inlined into the caller instead of
        # existing as its own labeled block).
        stub_cpp = os.path.join(tmp, "stub.cpp")
        with open(stub_cpp, "w") as f:
            f.write(f'#include "{header_path}"\n')
            f.write(f'extern "C" {{ volatile void* keep_compute_alive = (void*) &{case_name}::compute; }}\n')

        asm_path = os.path.join(tmp, "stub.s")
        compile_cmd = ["clang++", "-S", "-O3", "--std=c++20", "-I", SCRIPT_DIR, stub_cpp, "-o", asm_path]
        result = subprocess.run(compile_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"compile failed:\n{result.stderr}", file=sys.stderr)
            sys.exit(1)

        with open(asm_path) as f:
            full_asm = f.read()
        compute_asm = extract_compute_asm(full_asm, case_name)

        mca_cmd = ["/opt/homebrew/opt/llvm/bin/llvm-mca", f"-mcpu={mcpu}"]
        try:
            result = subprocess.run(mca_cmd, input=compute_asm, capture_output=True, text=True,
                                     timeout=MCA_TIMEOUT_SEC)
        except subprocess.TimeoutExpired:
            print(f"llvm-mca didn't finish within {MCA_TIMEOUT_SEC}s -- see the NOTE at the top "
                  f"of this script, this happened during development too", file=sys.stderr)
            sys.exit(1)

        if result.returncode != 0:
            print(f"llvm-mca failed:\n{result.stderr}", file=sys.stderr)
            sys.exit(1)

        return result.stdout


SUMMARY_FIELDS = {
    "Instructions":      ("instructions", int),
    "Total Cycles":      ("total_cycles", int),
    "Total uOps":        ("total_uops", int),
    "Dispatch Width":    ("dispatch_width", int),
    "uOps Per Cycle":    ("uops_per_cycle", float),
    "IPC":               ("ipc", float),
    "Block RThroughput": ("block_rthroughput", float),
}


def parse_summary(mca_output):
    """Pull the handful of summary fields out of llvm-mca's stdout -- these
    are what's worth folding into a perf JSON record; the per-instruction
    resource-pressure table further down is much more detailed than needed
    for a single scalar-comparison-across-experiments use case.
    """
    summary = {}
    for line in mca_output.splitlines():
        if ":" not in line:
            continue
        key, _, value = line.partition(":")
        key = key.strip()
        if key in SUMMARY_FIELDS:
            field_name, cast = SUMMARY_FIELDS[key]
            summary[field_name] = cast(value.strip())
    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("case", help="case name, e.g. rc_lowpass")
    parser.add_argument("--mcpu", default="apple-m1", help="passed through to llvm-mca -mcpu")
    parser.add_argument("--summary-only", action="store_true",
                         help="print just the parsed summary fields as JSON, not the full llvm-mca output")
    args = parser.parse_args()
    output = run_for_case(args.case, args.mcpu)
    if args.summary_only:
        import json
        print(json.dumps(parse_summary(output), indent=2))
    else:
        print(output)


if __name__ == "__main__":
    main()
