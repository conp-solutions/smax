#!/usr/bin/env bash
#
# Script to turn daimler cnf + cnf.amo
# Copyright (c) 2018, Norbert Manthey

# timeout in seconds to consider "interesting"
TIMEOUT=2

# maxsat solver to be used
MAXSAT=~/git/open-wbo/open-wbo_release

# create output directory
mkdir -p wcnf

# try 10 iterations
for loop in $(seq 1 10)
do
	# try for all formulas in this directory
	for f in *.cnf
	do
		echo $f
		b=$(basename $f)

		# turn CNF + AMOs into wcnf
		./create-selected-benchmark.py $f $f.amo wcnf > /tmp/wcnf.wcnf
		timeout $TIMEOUT $MAXSAT /tmp/wcnf.wcnf &> /dev/null
		e=$?
		S=$(awk '{print $4}' /tmp/wcnf.wcnf)
		echo "$f $e"
		if [ "$e" -eq 124 ]
		then
			mv /tmp/wcnf.wcnf maxsat/$b-$seed.wcnf
		fi
	done
done

# show if we created files
ls wcnf
