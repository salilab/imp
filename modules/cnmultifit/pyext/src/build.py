#!/usr/bin/env python

__doc__ = "Build cyclic symmetric complexes in their density map."

import IMP.cnmultifit
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <parameter file>

This program builds cyclic symmetric complexes in their density maps."""

    parser = OptionParser(usage)
    parser.add_option("--chimera", dest="chimera", default="", metavar="FILE",
                      help="the name of the Chimera output file, if desired")
    (options, args) = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    return args[0], options.chimera

def main():
    param_file, chimera_file = parse_args()
    IMP.base.set_log_level(IMP.WARNING)
    IMP.cnmultifit.do_all_fitting(param_file, chimera_file)

if __name__ == '__main__':
    main()
