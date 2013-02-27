#!/usr/bin/env python

__doc__ = "Show the DOMINO merge tree to be used in alignment."

import IMP.multifit
from optparse import OptionParser

def parse_args():
    usage =  """%prog [options] <asmb> <asmb.proteomics> <asmb.mapping>
           <alignment.params>

Show the DOMINO merge tree to be used in the alignment procedure
"""
    parser = OptionParser(usage)

    options, args = parser.parse_args()
    if len(args) !=4:
        parser.error("incorrect number of arguments")
    return options,args

def run(asmb_fn, proteomics_fn, mapping_fn, params_fn):
    asmb=IMP.multifit.read_settings(asmb_fn)
    asmb.set_was_used(True)
    dmap=IMP.em.read_map(asmb.get_assembly_header().get_dens_fn())
    dmap.get_header().set_resolution(
                         asmb.get_assembly_header().get_resolution())
    threshold=asmb.get_assembly_header().get_threshold()
    dmap.update_voxel_size(asmb.get_assembly_header().get_spacing())
    dmap.set_origin(asmb.get_assembly_header().get_origin())

    alignment_params = IMP.multifit.AlignmentParams(params_fn)
    alignment_params.show()
    IMP.base.set_log_level(IMP.WARNING)
    prot_data=IMP.multifit.read_proteomics_data(proteomics_fn)

    mapping_data=IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                           mapping_fn)

    em_anchors =  mapping_data.get_anchors()

    #load all proteomics restraints
    align=IMP.multifit.ProteomicsEMAlignmentAtomic(mapping_data,asmb,
                                                   alignment_params)

    align.set_fast_scoring(False)
    align.set_density_map(dmap,threshold)
    align.add_states_and_filters()
    align.add_all_restraints()
    print "\n\n\nDOMINO MERGE TREE\n\n"
    align.show_domino_merge_tree()

def main():
    options,args = parse_args()
    run(args[0], args[1], args[2], args[3])

if __name__ == "__main__":
    main()
