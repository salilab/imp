## Test that all atoms created by read_pbd get cleaned up when model is destroyed

import IMP.atom
import os.path
import sys

IMP.base.set_check_level(IMP.base.USAGE_AND_INTERNAL)
m = IMP.Model()
h = IMP.atom.read_pdb(os.path.join(os.path.split(sys.argv[0])[0], "input", "input.pdb"), m)

del m
if IMP.base.get_check_level() > IMP.base.NONE:
    lo = IMP.base.get_live_objects()
    print lo
    assert(len(lo) == 0)
