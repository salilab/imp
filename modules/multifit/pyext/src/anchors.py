#!/usr/bin/env python

__doc__ = "Generate anchors for a density map."

import IMP.multifit
import IMP.em
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <assembly input> <output anchors prefix>

Generate anchors for a density map."""
    parser = OptionParser(usage)
    parser.add_option("-s", "--size", type="int", dest="size", default=-1,
                      help="number of residues per bead")
    options, args = parser.parse_args()


    if len(args) != 2:
        parser.error("incorrect number of arguments")
    return options,args


def main():
    options,args=parse_args()
    asmb_fn=args[0]
    output=args[1]
    asmb = IMP.multifit.read_settings(asmb_fn)
    asmb.set_was_used(True)
    dmap = IMP.em.read_map(asmb.get_assembly_header().get_dens_fn(),
                           IMP.em.MRCReaderWriter())
    if options.size==-1:
        number_of_means = asmb.get_number_of_component_headers()
    else:
        total_num_residues=0
        mdl=IMP.Model()
        for i in range(asmb.get_number_of_component_headers()):
            total_num_residues+=len(IMP.atom.get_by_type(IMP.atom.read_pdb(asmb.get_component_header(i).get_filename(),mdl),IMP.atom.RESIDUE_TYPE))
            number_of_means=total_num_residues/options.size
    print "Calculating a segmentation into",number_of_means,"regions"
    density_threshold = asmb.get_assembly_header().get_threshold()

    IMP.multifit.get_anchors_for_density(dmap, number_of_means,
                                         density_threshold,
                                          output+".pdb", output+".cmm",
                                          "", output+".txt")

if __name__=="__main__":
    main()
