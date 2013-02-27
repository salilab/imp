#!/usr/bin/env python

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
           <alignment.params> <combinations[output]>
           <Fitting scores[output]>

Align proteomics graph with the EM map.
"""
    parser = OptionParser(usage)
    parser.add_option("-m", "--max", type="int", dest="max", default=999999999,
                      help="maximum number of fits considered")

    options, args = parser.parse_args()
    if len(args) !=6:
        parser.error("incorrect number of arguments")
    return options,args

def report_solutions(asmb, mdl, mhs, dmap, mapping_data, combs,
                     combs_fn_output_fn, scored_comb_output_fn, max_comb):
    ensmb = IMP.multifit.Ensemble(asmb, mapping_data)
    #report scores
    for i,mh in enumerate(mhs):
        fn = asmb.get_component_header(i).get_transformations_fn()
        ensmb.add_component_and_fits(mh,
                                     IMP.multifit.read_fitting_solutions(fn))
    all_leaves=[]
    for mh in mhs:
        mh_res=IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE)
        s1=IMP.atom.Selection(mh_res);
        s1.set_atom_types([IMP.atom.AtomType("CA")])
        all_leaves=all_leaves+s1.get_selected_particles();
    print "number of leaves:",len(all_leaves)
    print "Get number of restraints:",len(mdl.get_restraints())
    pb=progressBar(0,len(combs))
    fitr=IMP.em.FitRestraint(all_leaves,dmap)
    mdl.add_restraint(fitr)
    ranked_combs=[]
    sorted_combs=[]
    print "going to calculate fits for:",len(combs)
    for i,comb in enumerate(combs):
        if i%100 ==0:
            print "i:",i
        ensmb.load_combination(comb)
        ranked_combs.append([comb,fitr.evaluate(False)])
        ensmb.unload_combination(comb)
        pb.updateAmount(i)
        #print pb
    print "end fitting"
    # Sort by score
    ranked_combs.sort(lambda a,b: cmp(a[1], b[1]))
    # Remove excess combinations
    print "ranked combs:",len(ranked_combs)
    ranked_combs[max_comb:] = []
    print "ranked combs:",len(ranked_combs)
    for comb in ranked_combs:
        sorted_combs.append(comb[0])
    IMP.multifit.write_paths(sorted_combs,combs_fn_output_fn)
    output = open(scored_comb_output_fn,"w")
    for comb,score in ranked_combs:
        output.write("|")
        for ind in comb:
            output.write(str(ind)+" ")
        output.write("|"+str(1.-score)+"|\n")
    output.close()

def run(asmb_fn, proteomics_fn, mapping_fn, params_fn,
        combs_fn_output_fn, scored_comb_output_fn, max_comb):
    asmb=IMP.multifit.read_settings(asmb_fn)
    asmb.set_was_used(True)
    dmap=IMP.em.read_map(asmb.get_assembly_header().get_dens_fn())
    dmap.get_header().set_resolution(
                         asmb.get_assembly_header().get_resolution())
    threshold=asmb.get_assembly_header().get_threshold()
    dmap.update_voxel_size(asmb.get_assembly_header().get_spacing())
    dmap.set_origin(asmb.get_assembly_header().get_origin())
    #get rmsd for subunits
    print params_fn
    alignment_params = IMP.multifit.AlignmentParams(params_fn)
    alignment_params.show()
    IMP.base.set_log_level(IMP.WARNING)
    prot_data=IMP.multifit.read_proteomics_data(proteomics_fn)
    print "=========3"
    mapping_data=IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                           mapping_fn)
    print "=========4"
    em_anchors =  mapping_data.get_anchors()
    print "=========5"
    #load all proteomics restraints
    align=IMP.multifit.ProteomicsEMAlignmentAtomic(mapping_data,asmb,
                                                   alignment_params)
    print "align"
    align.set_fast_scoring(False)
    align.set_density_map(dmap,threshold)
    align.add_states_and_filters()
    align.add_all_restraints()

    print "before align"
    align.align()
    print "after align"
    combs=align.get_combinations()
    #print "after get combinations"
    if len(combs)==0:
        f=open(combs_fn_output_fn,"w")
        f.write("NO SOLUTIONS FOUND\n")
        f.close()
        sys.exit(0)

    report_solutions(asmb, align.get_model(), align.get_molecules(), dmap,
                     mapping_data, combs, combs_fn_output_fn,
                     scored_comb_output_fn, max_comb)

def main():
    options,args = parse_args()
    run(args[0], args[1], args[2], args[3], args[4], args[5],
        options.max)
if __name__ == "__main__":
    main()
