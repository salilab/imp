#!/usr/bin/python
import sys
sys.argv.sort()
for h in sys.argv:
    if h.endswith(".h"):
        print "#include <RMF/"+h[h.rfind("RMF")+4:]+">"
