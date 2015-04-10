#!/usr/bin/env python

"""Replace all instances of argv[1] with argv[2], only updating the file if
   changes were made. """

import sys
import os

if len(sys.argv) < 3:
    print("usage:", sys.argv[0], " find_string replace_string files....")
    exit(1)

instring = sys.argv[1]
outstring = sys.argv[2]
files = sys.argv[3:]

for f in files:
    if os.path.isdir(f):
        continue
    contents = open(f, "r").read()
    if contents.find(instring) != -1:
        open(f, "w").write(contents.replace(instring, outstring))
