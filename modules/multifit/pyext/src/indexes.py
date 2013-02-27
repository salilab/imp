#!/usr/bin/env python

__doc__ = "Generate indexes of fitting solutions."

import IMP.multifit
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <assembly name> <assembly input> <number of fits> <indexes filename>

Generate indexes of fitting solutions.
"""
    parser = OptionParser(usage)
    options, args = parser.parse_args()
    if len(args) != 4:
        parser.error("incorrect number of arguments")
    return options,args

def run(assembly_name,asmb_fn,num_fits,mapping_fn=""):
    IMP.base.set_log_level(IMP.WARNING)
    asmb_input=IMP.multifit.read_settings(asmb_fn)
    asmb_input.set_was_used(True)
    ap_em_fn=asmb_input.get_assembly_header().get_coarse_over_sampled_ap_fn()
    if mapping_fn=="":
        mapping_fn=assembly_name+".indexes.mapping.input"
    mapping_data=open(mapping_fn,"w")
    mapping_data.write("|anchors|"+ap_em_fn+"|\n")
    for i in range(asmb_input.get_number_of_component_headers()):
        name=asmb_input.get_component_header(i).get_name()
        trans_fn = asmb_input.get_component_header(i).get_transformations_fn()
        fits=IMP.multifit.read_fitting_solutions(trans_fn)
        print "number of fits for component",i,"is",len(fits),trans_fn
        index_fn=assembly_name+"."+name+".fit.indexes.txt"
        indexes=[]
        for i in range(min(num_fits,len(fits))):
            indexes.append([i])
        IMP.multifit.write_paths(indexes,index_fn)
        mapping_data.write("|protein|"+name+
                           "|"+index_fn+"|\n")
    mapping_data.close()

def main():
    options,args = parse_args()
    asmb_name = args[0]
    asmb_input = args[1]
    max_fits = int(args[2])
    mapping_fn=args[3]
    run(asmb_name,asmb_input,max_fits,mapping_fn)

if __name__=="__main__":
    main()
