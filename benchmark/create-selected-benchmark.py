#!/usr/bin/env python
#
# Script to turn CNF into MaxSAT / Group MUS formula
# Copyright (c) 2018, Norbert Manthey

import sys
import random

# print usage of this script
def usage():
    print "Script to create a MaxSAT or Group MUS formula based on a CNF and a set of selector sets"
    print "From some selector (eventually encoded as AMO in the input CNF)"
    print "Prints the formula to stdout"
    print ""
    print "Usage: {} {} {} {} [{} [{}]".format(
              "create-selected-benchmark.py",
              "<input.cnf>",
              "<input.cnf.amos>",
              "<gcnf/wcnf>",
              "seed",
              "fraction")
    print "input.cnf       ... CNF base formula, will be used as hard formula in the final formula"
    print "input.cnf.amos  ... lines of literals from which one could be selected, could be encoded AMOs in the input formula"
    print "gcnf/wcnf       ... create Group-MUS or (partial weighted) MaxSAT formula"
    print "seed            ... initialization for random number generator (is produced, if not specified)"
    print "fraction        ... amount of selectors that will be used, and will be chosen randomly (is produced, if not specified)"


# print usage in case no parameters are given
if(len(sys.argv) == 1):
    usage()
    sys.exit(0)

# input files
inputcnf = sys.argv[1]
inputamo = sys.argv[2]

# Group MUS or MaxSAT
gcnf = False
if sys.argv[3] == "gcnf":
    gcnf = True
elif sys.argv[3] != "wcnf":
        print "did not recognize gcnf/wcnf parameter: {}, abort".format(sys.argv[3])
        sys.exit(1)

# if there is a seed, consume it, otherwise, produce a seed
if(len(sys.argv) >= 5):
    seed = int(sys.argv[4])
else:
    seed = random.randint(0, 2 ** 30)

random.seed(seed)

# how many of the given constraints should be used to pick something
if len(sys.argv) < 5:
    fraction = random.uniform(0.05, 1)
else:
    fraction = float(sys.argv[5])

# in case of MaxSAT, pick a rather small range for weights
maxweight = 1
if not gcnf:
	# partial or weighted partial maxsat?, and maximum weight
	maxweight = random.randint(1, 4)

# consume input files
amos = []
with open(inputamo) as f:
    lines = f.readlines()
    for line in lines:
        amos.append(line.split())

with open(inputcnf) as f:
    lines = f.readlines()

use_amo = int(float(len(amos)) * fraction)

# collect the top weight
weights = []
weight_sum = 0
for e in range(0, use_amo):
    weight = random.randint(1, maxweight)
    weight_sum += weight
    weights.append(random.randint(1, maxweight))
top_weight = weight_sum + 1

# prefix for all clauses in the hard formula, which represents the original CNF
hard_prefix = ""
if gcnf:
    hard_prefix = "{0}"
else:
    hard_prefix = str(top_weight)

# print CNF with extra head
print "c generated AMO with {}".format(" ".join(sys.argv))
print "c used seed: {}".format(seed)
for line in lines:
    line = line.rstrip()
    if line.startswith("p cnf"):
        items = line.split()
        cls = int(items[3])
        if gcnf:
            # each AMO from which we select represents a group
            print "p gcnf {} {} {}".format(items[2], cls + use_amo, use_amo)
        else:
            # there are additional unit clauses with weights, any other clause is a hard clause
            print "p wcnf {} {} {}".format(items[2], cls + use_amo, top_weight)
        # do not print the original cnf line
        continue

    # print the core CNF formula
    if line:
        print "{} {}".format(hard_prefix, line)

# make sure we do not always pick the first AMOs
random.shuffle(amos)

# print selected units in the format for either maxsat or group MUS
i = 0
for amo in amos:
    if i >= use_amo:
        break

    prefix = ""
    if gcnf:
        prefix = "{" + str(i+1) + "}"
    else:
        prefix = str(weights[i])

    i += 1
    selected_item = random.randint(0, len(amo) - 1)
    # print >> sys.stderr, "c chose {} out of {} ({})".format(selected_item, len(amo) - 1, amo)
    print "{} {} 0".format(prefix, amo[selected_item])
