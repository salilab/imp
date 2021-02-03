#!/usr/bin/python3

"""
Simple script to generate a cubic spline statistical potential file in
IMP format (see help for IMP::score_functor::Statistical and output files
in modules/score_functor/data/ such as dope_score.lib) from the equivalent
MODELLER files. It takes two arguments - paths to the MODELLER atom class
file and the MODELLER distances file - and outputs suitable IMP Key
definitions (see modules/score_functor/src/Dope.cpp) and IMP potential file,
e.g.

statistical_from_modeller.py atmcls-melo.lib melo1-dist.lib
"""

import sys

atmgrp = None
with open(sys.argv[1]) as fh:
    for line in fh:
        spl = line.strip().split()
        if spl[0] == 'ATMGRP':
            atmgrp = spl[1].replace("'", '')
            print("LOOP_TYPE(%s)" % atmgrp)
        elif spl[0] == 'ATOM':
            print("LOOP_TYPE_ALIAS(%s_%s, %s)"
                  % (spl[1].replace("'", ''), spl[2].replace("'", ''), atmgrp))


with open(sys.argv[2]) as fh:
    lines = fh.readlines()

if lines[0].rstrip('\r\n') != 'MOD5':
    raise ValueError("First line is %s, not MOD5" % lines[0].rstrip('\r\n'))

atom_classes = set()
for line in lines[1:]:
    spl = line.split()
    low, high, width, lowder, highder = [float(x) for x in spl[11:16]]
    if spl[:2] != ['R', '10']:
        raise ValueError("Restraint line %s does not start with R 10"
                         % line.rstrip('\r\n'))
    at1, at2 = spl[8], spl[9]
    atom_classes.add(at1)
    atom_classes.add(at2)

n_atom_classes = len(atom_classes)
exp_len_lines = n_atom_classes * (n_atom_classes+1) // 2 + 1
if len(lines) != exp_len_lines:
    raise ValueError("File does not contain all N vs M interactions "
                     "for %d classes; expecting %d lines but saw %d"
                     % (n_atom_classes, exp_len_lines, len(lines)))

# Modeller potential is for score values at the low edge of each bin;
# IMP uses the center, so we need to offset by half the bin width
print("%f %d %f" % (width, n_atom_classes, low - width/2))

for line in lines[1:]:
    spl = line.split()
    at1, at2 = spl[8], spl[9]
    values = [float(x) for x in spl[16:]]
    print("%8s %8s  " % (at1, at2) + " ".join("%7.4f" % x for x in values))
