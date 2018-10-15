#!/usr/bin/env bash
# Norbert Manthey, Copyright 2018, All rights reserved
#
# Script to check smax implementation via valgrind

# make sure we see what is executed, and fail early
set -e -x

# go to this directory
script_dir=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# build the debug version
if [ -d "$script_dir"/../../smax-src ] && [ -z "$VALGRIND_NO_REBUILD" ]
then
	pushd "$script_dir"/../../smax-src
	make clean
	pwd; make smax_debug -j $(nproc)
	popd
fi

CHECK_STATUS=0

# make sure we are working where the cnfs are located
cd "$script_dir"

BENCHMARKS=$(ls benchmarks/*cnf || true)

# let's iterate over all files
for file in *.cnf $BENCHMARKS
do

	echo "check file $file"
	b=$(basename $file)

	# check single core, and disjunct cores
	for parameter in ""
	do
		LOGFILE="$b-$parameter.log"

		# run tool via valgrind
		valgrind -v \
			--track-origins=yes \
			--leak-check=full \
			--show-leak-kinds=all \
			"$script_dir"/../../smax-src/smax_debug \
				-verb=2 $parameter \
				$file |& tee "$LOGFILE"

		# make sure we do not have any errors
		grep "ERROR SUMMARY: 0 errors" "$LOGFILE" || CHECK_STATUS=1

		echo "for 2 $CHECK_STATUS"
		[ $CHECK_STATUS -eq 0 ] || break
	done
	echo "for 1 $CHECK_STATUS"
	[ $CHECK_STATUS -eq 0 ] || break

done | tee valgrind-benchmark.log

# return back to calling directory
cd -
