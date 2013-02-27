#!/usr/bin/env python

__doc__ = "Calculate RMSD between a model and the reference."

import IMP.cnmultifit
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <parameter file> <transformations file>
        <reference PDB>

This program calculates the RMSD between modeled cyclic symmetric complexes and
the reference structure. The RMSD and cross correlation of each complex is
written into a file called rmsd.output.

Notice: no structural alignment is performed!"""

    parser = OptionParser(usage)
    parser.add_option("--vec", dest="vec", default="", metavar="FILE",
                      help="output the RMSDs as a vector into the named "
                           "file, if specified")
    parser.add_option("--start", dest="start", default=0, type="int",
                      help="first model in transformations file to compare "
                           "with the reference (by default, model 0)")
    parser.add_option("--end", dest="end", default=-1, type="int",
                      help="last model in transformations file to compare "
                           "with the reference (by default, the final model)")
    (options, args) = parser.parse_args()
    if len(args) != 3:
        parser.error("incorrect number of arguments")
    return options, args

def main():
    opts, args = parse_args()
    IMP.base.set_log_level(IMP.WARNING)
    rmsds = IMP.cnmultifit.get_rmsd_for_models(args[0], args[1], args[2],
                                               opts.start, opts.end)
    if opts.vec:
        open(opts.vec, 'w').write(" ".join(['%f' % x for x in rmsds]))

if __name__ == '__main__':
    main()
