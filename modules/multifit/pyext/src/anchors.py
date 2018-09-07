#!/usr/bin/env python

from __future__ import print_function
import IMP.multifit
import IMP.em
from IMP import ArgumentParser

__doc__ = "Generate anchors for a density map."

def parse_args():
    desc = """Generate anchors for a density map."""
    p = ArgumentParser(description=desc)
    p.add_argument("-s", "--size", type=int, dest="size", default=-1,
                   help="number of residues per bead")
    p.add_argument("assembly_file", help="assembly file name")
    p.add_argument("anchor_prefix", help="prefix for output anchors file names")

    return p.parse_args()


def main():
    args = parse_args()
    output = args.anchor_prefix
    asmb = IMP.multifit.read_settings(args.assembly_file)
    asmb.set_was_used(True)
    dmap = IMP.em.read_map(asmb.get_assembly_header().get_dens_fn(),
                           IMP.em.MRCReaderWriter())
    if args.size == -1:
        number_of_means = asmb.get_number_of_component_headers()
    else:
        total_num_residues = 0
        mdl = IMP.Model()
        for i in range(asmb.get_number_of_component_headers()):
            total_num_residues += len(
                IMP.atom.get_by_type(
                    IMP.atom.read_pdb(
                        asmb.get_component_header(
                            i).get_filename(
                        ),
                        mdl),
                    IMP.atom.RESIDUE_TYPE))
            number_of_means = total_num_residues / args.size
    print("Calculating a segmentation into", number_of_means, "regions")
    density_threshold = asmb.get_assembly_header().get_threshold()

    IMP.multifit.get_anchors_for_density(dmap, number_of_means,
                                         density_threshold,
                                         output + ".pdb", output + ".cmm",
                                         "", output + ".txt")

if __name__ == "__main__":
    main()
