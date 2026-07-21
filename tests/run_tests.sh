#!/bin/bash

set -e

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

if [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
   python_exe="python"
else
   python_exe="python3"
fi

test () {
   test="$1"
   ${python_exe} ${SCRIPT_DIR}/${test}/test.py
}

# simple circuits
test rc_lowpass
test res_highpass
test sk_lpf
test summing_amp

# linear circuits
test reductions
test eq_filter
test eq_filter2
test multi_svf

# simple nonlinear
test diode_circuit
test diode_clipper
test common_emitter
test common_drain

# guitar pedal circuits
test pedal_tone
test pedal_drive
test pedal_model
