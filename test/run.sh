#!/usr/bin/env bash
# Norbert Manthey, Copyright 2018, All rights reserved
#
# Run all testing in one go

# make sure we see what is executed, and fail early
set -e -x

# go to this directory
script_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

cd "$script_dir"

# biuld smoother files
pushd "$script_dir"/..
make clean
make
popd

# check whether each test can be build
for f in $(ls * -d)
do
	[ -d "$f" ] || continue
	[ -f "$f"/Makefile ] || continue

	echo "build test $f"
	pushd "$f"
        make clean
	make
        make clean
	popd
done

# check whether all tests passed
echo "all testing: PASSED"
