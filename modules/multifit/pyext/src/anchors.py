#!/usr/bin/python

__doc__ = "Generate anchors for a density map."

import IMP.multifit
import IMP.em
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <assembly input> <output anchors prefix>

Generate anchors for a density map."""
    parser = OptionParser(usage)
    options, args = parser.parse_args()
    if len(args) != 2:
        parser.error("incorrect number of arguments")
    return args

def main():
    asmb_fn, output = parse_args()
    asmb = IMP.multifit.read_settings(asmb_fn)
    asmb.set_was_used(True)
    dmap = IMP.em.read_map(asmb.get_assembly_header().get_dens_fn(),
                           IMP.em.MRCReaderWriter())
    number_of_means = asmb.get_number_of_component_headers()
    density_threshold = asmb.get_assembly_header().get_threshold()

    IMP.multifit.get_anchors_for_density(dmap, number_of_means,
                                         density_threshold,
                                          output+".pdb", output+".cmm",
                                          "", output+".txt")

if __name__=="__main__":
    main()
