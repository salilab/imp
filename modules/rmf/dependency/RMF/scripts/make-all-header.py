#!/usr/bin/python
import sys
for h in sys.argv:
    if h.endswith(".h"):
        print "#include <RMF/"+h[h.rfind("RMF")+4:]+">"
