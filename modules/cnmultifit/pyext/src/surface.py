#!/usr/bin/env python

__doc__ = "Generate a Connolly surface for a PDB."

import IMP.atom
import IMP.multifit
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <pdb file name>

This program generates the Connolly surface for a given PDB file."""

    parser = OptionParser(usage)
    parser.add_option("--density", dest="density", default=10.0, type="float",
                      metavar="D",
                      help="density of probe points, per cubic angstrom "
                           "(default 10.0)")
    parser.add_option("--radius", dest="rp", default=1.8, type="float",
                      metavar="R",
                      help="probe radius in angstroms (default 1.8)")

    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error("incorrect number of arguments")
    return args[0], opts.density, opts.rp

def main():
    infile, density, rp = parse_args()
    outfile = infile + '.ms'

    m = IMP.Model()
    h = IMP.atom.read_pdb(infile, m,
                          IMP.atom.NonWaterNonHydrogenPDBSelector())
    IMP.multifit.write_connolly_surface(IMP.atom.get_leaves(h),
                                        outfile, density, rp)

if __name__ == "__main__":
    main()
