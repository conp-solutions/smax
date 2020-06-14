#!/bin/bash

# make sure we see what is executed, and fail early
set -e -x

# go to this directory
script_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# rebuild solver, if not present, and we're in the correct place
if [ -r "$script_dir"/../../smax-src/Makefile ] && [ ! -x "$script_dir"/../../smax-src/smax_static ]
then
        pushd "$script_dir"/../..
        make smax
        popd
fi

SOLVER=$(readlink -e "$script_dir"/../../smax-src/smax_static)

rm -rf maxsat-fuzzer
git clone https://github.com/conp-solutions/maxsat-fuzzer.git

cd maxsat-fuzzer
make

STATUS=0
./fuzz.py 50 "$SOLVER" || STATUS=$?

exit $STATUS
