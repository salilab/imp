#!/usr/bin/env python

__doc__ = "Generate a Connolly surface for a PDB."

import IMP.atom
import IMP.multifit
from IMP import ArgumentParser


def parse_args():
    desc = """
This program generates the Connolly surface for a given PDB file."""

    p = ArgumentParser(description=desc)
    p.add_argument("--density", dest="density", default=10.0, type=float,
                   metavar="D",
                   help="density of probe points, per cubic angstrom "
                        "(default 10.0)")
    p.add_argument("--radius", dest="rp", default=1.8, type=float,
                   metavar="R",
                   help="probe radius in angstroms (default 1.8)")
    p.add_argument("pdb", help="input PDB file name")

    args = p.parse_args()
    return args.pdb, args.density, args.rp


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
