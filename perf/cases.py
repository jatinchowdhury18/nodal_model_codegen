"""The representative circuits used by the perf harness. They span
small/large and linear/nonlinear without every run taking forever.
"""

CASES = [
    {"name": "rc_lowpass",     "net": "rc_lowpass/rc_lowpass.net",         "dtype": "float"},
    {"name": "eq_filter2",     "net": "eq_filter2/eq_filter2.net",         "dtype": "float"},
    {"name": "diode_clipper",  "net": "diode_clipper/diode_clipper.net",   "dtype": "float"},
    {"name": "common_emitter", "net": "common_emitter/common_emitter.net", "dtype": "float"},
    # pedal_drive needs double -- the coupled 3-unknown cluster's Cramer's-rule
    # closed forms lose too much precision in float.
    {"name": "pedal_drive",    "net": "pedal_drive/pedal_drive.net",       "dtype": "double"},
    # Big, fully-linear circuit (1073-style EQ, split into shelf/bell/hpf
    # sub-circuits). This is the one that needed hand-rewriting for register
    # pressure; see scratch/PERF_PLAN.md.
    {"name": "eq73",           "net": "eq73/eq73.net",                     "dtype": "float"},
]

CASE_NAMES = [c["name"] for c in CASES]
