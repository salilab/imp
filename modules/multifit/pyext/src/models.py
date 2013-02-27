#!/usr/bin/env python

__doc__ = "Write output models as PDB files."

#analyse the ensemble, first we will do the rmsd stuff
import IMP.multifit
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <asmb.input> <proteomics.input>
           <mapping.input> <combinations> <model prefix>

Write output models.
"""
    parser = OptionParser(usage)
    parser.add_option("-m", "--max", type="int", dest="max", default=None,
                      help="maximum number of models to write")
    (options, args) = parser.parse_args()
    if len(args) != 5:
        parser.error("incorrect number of arguments")
    return options,args

def run(asmb_fn,proteomics_fn,mapping_fn,combs_fn,model_output,max_comb):
    #get rmsd for subunits
    mdl=IMP.Model()
    combs=IMP.multifit.read_paths(combs_fn)
    sd=IMP.multifit.read_settings(asmb_fn)
    sd.set_was_used(True)
    prot_data=IMP.multifit.read_proteomics_data(proteomics_fn)
    mapping_data=IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                           mapping_fn)
    ensmb=IMP.multifit.load_ensemble(sd,mdl,mapping_data)
    mhs=ensmb.get_molecules()
    print "number of combinations:",len(combs),max_comb
    for i,comb in enumerate(combs[:max_comb]):
        if i%500==0:
            print i
        ensmb.load_combination(comb)
        print model_output+"."+str(i)+".pdb"
        IMP.atom.write_pdb(mhs,model_output+"."+str(i)+".pdb")
        ensmb.unload_combination(comb)

def main():
    options,args = parse_args()
    run(args[0],args[1],args[2],args[3],args[4],options.max)

if __name__ == "__main__":
    main()
