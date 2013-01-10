#!/usr/bin/python

__doc__ = "Align proteomics graph with the EM map."

#analyse the ensemble, first we will do the rmsd stuff
import sys
import IMP.multifit
from optparse import OptionParser

class progressBar:
    def __init__(self, minValue = 0, maxValue = 10, totalWidth=12):
        self.progBar = "[]"   # This holds the progress bar string
        self.min = minValue
        self.max = maxValue
        self.span = maxValue - minValue
        self.width = totalWidth
        self.amount = 0       # When amount == max, we are 100% done
        self.updateAmount(0)  # Build progress bar string

    def updateAmount(self, newAmount = 0):
        if newAmount < self.min: newAmount = self.min
        if newAmount > self.max: newAmount = self.max
        self.amount = newAmount

        # Figure out the new percent done, round to an integer
        diffFromMin = float(self.amount - self.min)
        percentDone = (diffFromMin / float(self.span)) * 100.0
        percentDone = round(percentDone)
        percentDone = int(percentDone)

        # Figure out how many hash bars the percentage should be
        allFull = self.width - 2
        numHashes = (percentDone / 100.0) * allFull
        numHashes = int(round(numHashes))

        # build a progress bar with hashes and spaces
        self.progBar = "[" + '#'*numHashes + ' '*(allFull-numHashes) + "]"

        # figure out where to put the percentage, roughly centered
        percentPlace = (len(self.progBar) / 2) - len(str(percentDone))
        percentString = str(percentDone) + "%"

        # slice the percentage into the bar
        self.progBar = self.progBar[0:percentPlace] + percentString + self.progBar[percentPlace+len(percentString):]

    def __str__(self):
        return str(self.progBar)


def parse_args():
    usage =  """%prog [options] <asmb> <asmb.proteomics> <asmb.mapping>
           <alignment.params>

view domino merge tree to be used in the alignment procedure
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
    #get rmsd for subunits

    alignment_params = IMP.multifit.AlignmentParams(params_fn)

    alignment_params.process_parameters()

    alignment_params.show()
    IMP.set_log_level(IMP.WARNING)
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
    align.view_domino_merge_tree()

def main():
    options,args = parse_args()
    run(args[0], args[1], args[2], args[3])

if __name__ == "__main__":
    main()
