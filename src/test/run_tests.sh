#!/bin/bash

set -e
shopt -s expand_aliases
source ~/.bashrc

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
cd $SCRIPT_DIR

rm -f *.exe

jai build.jai

for file in *.exe; do
    echo "Running test: $file"
    ./$file
    # Your commands here (always double-quote "$file" to handle spaces)
done
