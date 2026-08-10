"""The 5 representative circuits used by the perf harness -- span
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
]

CASE_NAMES = [c["name"] for c in CASES]
