#!/usr/bin/env python

__doc__ = "Calculate RMSD between a model and the reference."

import IMP.cnmultifit
from IMP import ArgumentParser


def parse_args():
    desc = """%prog [options] <parameter file> <transformations file>
        <reference PDB>

This program calculates the RMSD between modeled cyclic symmetric complexes and
the reference structure. The RMSD and cross correlation of each complex is
written into a file called rmsd.output.

Notice: no structural alignment is performed!"""

    p = ArgumentParser(description=desc)
    p.add_argument("--vec", dest="vec", default="", metavar="FILE",
                   help="output the RMSDs as a vector into the named "
                        "file, if specified")
    p.add_argument("--start", dest="start", default=0, type=int,
                   help="first model in transformations file to compare "
                        "with the reference (by default, model 0)")
    p.add_argument("--end", dest="end", default=-1, type=int,
                   help="last model in transformations file to compare "
                        "with the reference (by default, the final model)")
    p.add_argument("param_file", help="parameter file name")
    p.add_argument("trans_file", help="transformations file name")
    p.add_argument("ref_pdb", help="reference PDB file name")

    return p.parse_args()


def main():
    IMP.set_log_level(IMP.WARNING)
    args = parse_args()
    rmsds = IMP.cnmultifit.get_rmsd_for_models(args.param_file,
                                               args.trans_file, args.ref_pdb,
                                               args.start, args.end)
    if args.vec:
        open(args.vec, 'w').write(" ".join(['%f' % x for x in rmsds]))

if __name__ == '__main__':
    main()
