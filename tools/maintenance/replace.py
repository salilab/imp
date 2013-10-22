#! /usr/bin/python

import sys

instring = sys.argv[1]
outstring = sys.argv[2]
files = sys.argv[3:]

for f in files:
    contents = open(f, "r").read()
    if contents.find(instring) != -1:
        open(f, "w").write(contents.replace(instring, outstring))
