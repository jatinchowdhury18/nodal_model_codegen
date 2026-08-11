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
test eq_filter
test eq_filter2
test multi_svf
test eq73

# simple nonlinear
test diode_circuit
test diode_clipper
test common_emitter
test common_emitter_pnp
test common_drain
test common_drain_pjf
test common_cathode

# "real world" nonlinear circuits
test pedal_tone
test pedal_drive
test pedal_model
test tube_compressor

# op-amp non-idealities
test opamp_finite_gain
test opamp_clipping
test opamp_complete

# pathological test circuits
test reductions
test rc_cascade_stability
test dc_coupled_x2
