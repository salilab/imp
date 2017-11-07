# Test that all atoms created by read_pbd get cleaned up when model is
# destroyed

import IMP.atom
import os.path
import sys

IMP.set_check_level(IMP.USAGE_AND_INTERNAL)
IMP.set_log_level(IMP.WARNING)
m = IMP.Model()
h = IMP.atom.read_pdb(
    os.path.join(os.path.split(__file__)[0],
                 "input",
                 "input.pdb"),
    m)

del m
