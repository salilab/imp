#!/usr/bin/python
import sys
import glob
import os
for h in sys.argv[2:]:
    pat = os.path.join(h, "*.h")
    for g in glob.glob(pat):
        name= os.path.split(g)[1]
        print "#include <%s/"%sys.argv[1] + name + ">"
