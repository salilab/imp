## Test that all atoms created by read_pbd get cleaned up when model is destroyed

import IMP.atom
import os.path
import sys

IMP.base.set_check_level(IMP.base.USAGE_AND_INTERNAL)
IMP.base.set_log_level(IMP.base.WARNING)
m = IMP.Model()
h = IMP.atom.read_pdb(os.path.join(os.path.split(sys.argv[0])[0], "input", "input.pdb"), m)

del m
