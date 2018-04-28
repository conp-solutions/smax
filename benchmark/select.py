#!/usr/bin/env python
import sys
import random

inputcnf = sys.argv[1]
inputamo = sys.argv[2]
seed = int(sys.argv[3])

#print >> sys.stderr, "c set seed {}".format(seed)
random.seed(seed)

if len(sys.argv) < 5:
    fraction = random.uniform(0.05, 1)
else:
    fraction = float(sys.argv[4])

amos = []
with open(inputamo) as f:
    # print >> sys.stderr, "c parse file {}".format(inputamo)
    lines = f.readlines()
    for line in lines:
        amos.append(line.split())

#print >> sys.stderr, "c parse cnf {}".format(inputcnf)
with open(inputcnf) as f:
    lines = f.readlines()

use_amo = int(float(len(amos)) * fraction)

# print CNF with extra head
print "c generated AMO with {}".format(" ".join(sys.argv))
for line in lines:
    line = line.rstrip()
    if line.startswith("p cnf"):
        items = line.split()
        cls = int(items[3])
        print "p cnf {} {}".format(items[2], cls + use_amo),
        continue

    if line:
        print line

random.shuffle(amos)

# print selected units
i = 0
for amo in amos:
    if i >= use_amo:
        break
    i += 1
    selected_item = random.randint(0, len(amo) - 1)
    # print >> sys.stderr, "c chose {} out of {} ({})".format(selected_item, len(amo) - 1, amo)
    print "{} 0".format(amo[selected_item])

