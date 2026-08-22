"""The representative circuits used by the perf harness. They span
small/large and linear/nonlinear without every run taking forever.

A case may carry an optional "extra_args" list of extra netlist_codegen CLI
flags, for a flag that only makes sense for that one circuit. To sweep a flag
across every case instead, pass `--codegen-arg` to run_perf.py rather than
editing this file: the harness deliberately knows nothing about which codegen
flags exist.
"""

CASES = [
    {"name": "rc_lowpass",     "net": "rc_lowpass/rc_lowpass.net",         "dtype": "float"},
    {"name": "eq_filter2",     "net": "eq_filter2/eq_filter2.net",         "dtype": "float"},
    {"name": "diode_clipper",  "net": "diode_clipper/diode_clipper.net",   "dtype": "float"},
    {"name": "common_emitter", "net": "common_emitter/common_emitter.net", "dtype": "float"},
    {"name": "pedal_drive",    "net": "pedal_drive/pedal_drive.net",       "dtype": "double"},
    # Tests nonlinear cluster de-coupling.
    {"name": "pedal_model",    "net": "pedal_model/pedal_model.net",       "dtype": "double"},
    # Largest nonlinear cluster in the fixture set (k=5), so it dominates the
    # per-iteration and iteration-count cost model.
    {"name": "tube_compressor", "net": "tube_compressor/tube_compressor.net", "dtype": "double"},
    # Big, fully-linear circuit (1073-style EQ, split into shelf/bell/hpf
    # sub-circuits). This is the one that needed hand-rewriting for register
    # pressure; see scratch/PERF_PLAN.md.
    {"name": "eq73",           "net": "eq73/eq73.net",                     "dtype": "float"},
]

CASE_NAMES = [c["name"] for c in CASES]
