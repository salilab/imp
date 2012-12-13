#!/usr/bin/python

#This file is generate automatically from the anchor graph and fitting results.
#No interaction data is entered here, can be entered by the user
#read the anchors
#read the top fit for each protein, and assign the anchors
#add EV accordinly
import IMP.multifit
import sys
import IMP.atom

def usage():
    if len(sys.argv) != 3:
        print sys.argv[0]+" <asmb.input> <output:proteomics>"
        sys.exit(1)
def run(asmb_fn,proteomics_fn):
    asmb=IMP.multifit.read_settings(asmb_fn)
    asmb.set_was_used(True)
    outf=open(proteomics_fn,"w")
    outf.write("|proteins|\n")
    mdl=IMP.Model()
    for i in range(asmb.get_number_of_component_headers()):
        prot_name=asmb.get_component_header(i).get_name()
        prot_fn=asmb.get_component_header(i).get_filename()
        prot=IMP.atom.read_pdb(prot_fn,mdl)
        outf.write("|"+asmb.get_component_header(i).get_name()+"|1|"+str(len(IMP.atom.get_by_type(prot,IMP.atom.RESIDUE_TYPE)))+"|nn|nn|\n")
    outf.write("|interactions|\n")
    outf.write("|residue-xlink|\n")
    outf.write("|ev-pairs|\n")
    outf.close()
if __name__=="__main__":
    usage()
    run(sys.argv[1],sys.argv[2])
