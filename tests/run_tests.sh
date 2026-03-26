#!/bin/bash

set -e

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

test () {
   test="$1"
   python3 ${SCRIPT_DIR}/${test}/test.py
}

test rc_lowpass
test res_highpass
test sk_lpf
