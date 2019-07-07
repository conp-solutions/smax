#!/usr/bin/env bash
# Norbert Manthey, Copyright 2018, All rights reserved
#
# Simple script to show case using dynamically shared library

# make sure we see what is executed, and fail early
set -e -x

# go to this directory
script_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# build shared library if we have source (no-op in case already present)
if [ -r "$script_dir"/../../smax-src/Makefile ]
then
	pushd "$script_dir"/../..
	make libs
	popd
fi

pushd "$script_dir"

# run test with statically linked library, if present
if [ -f ../../lib/libsmax.a ]
then
g++ maxsat-test.cc -I../.. -L../../lib -lsmax -std=c++11 -lz -lgmp -o maxsat-test -static --coverage
./maxsat-test
fi

# build maxsat-test, with:
#  -I../..               smax base path for includes
#  -L../../smax-src      smax-src path to look for libraries
#  -lsmax                actually link against libsmax.so file
#  -std=c++11            use c++11 standart, because MaxSATSolver would not compile otherwise
#  -lz                   link against libz library, as required by the SAT solver
#  -o maxsat-test        name the binary maxsat-test
g++ maxsat-test.cc -I../.. -L../../lib -lsmax -std=c++11 -lz -lgmp -o maxsat-test --coverage

# check whether dynamic libraries can be found
LD_LIBRARY_PATH=../../lib ldd ./maxsat-test

# actually run the maxsat-test with the shared dynamic library
LD_LIBRARY_PATH=../../lib ./maxsat-test

# return back to calling directory
popd

echo "SUCCESS!"
