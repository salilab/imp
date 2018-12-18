#!/usr/bin/env python

__doc__ = "Build cyclic symmetric complexes in their density map."

import IMP.cnmultifit
from IMP import ArgumentParser


def parse_args():
    desc = """
    This program builds cyclic symmetric complexes in their density maps."""

    parser = ArgumentParser(description=desc)
    parser.add_argument("--chimera", dest="chimera", default="", metavar="FILE",
                      help="the name of the Chimera output file, if desired")
    parser.add_argument("param_file", help="parameter file name")
    return parser.parse_args()


def main():
    IMP.set_log_level(IMP.WARNING)
    args = parse_args()
    IMP.cnmultifit.do_all_fitting(args.param_file, args.chimera)

if __name__ == '__main__':
    main()
