#!/usr/bin/env python
import IMP.multifit
import sys,os
def usage():
    if len(sys.argv) != 3:
        print sys.argv[0]+" <num of rotations to sample> <output file>"
        sys.exit()
if __name__=="__main__":
    usage()
    rots=IMP.algebra.get_uniform_cover_rotations_3d(int(sys.argv[1]))
    vec0=IMP.algebra.Vector3D(0,0,0)
    trans=IMP.multifit.FittingSolutionRecords()
    for i,r in enumerate(rots):
        rec=IMP.multifit.FittingSolutionRecord()
        rec.set_index(i);
        rec.set_fit_transformation(IMP.algebra.Transformation3D(r,vec0))
        trans.push_back(rec)
    IMP.multifit.write_fitting_solutions(sys.argv[2],trans);
