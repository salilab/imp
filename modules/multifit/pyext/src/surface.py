#!/usr/bin/python

__doc__ = "Generate Connolly surface for all subunits."

import IMP.multifit
import IMP.atom
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <assembly input>

Create surface for all assembly proteins."""
    parser = OptionParser(usage)
    options, args = parser.parse_args()
    if len(args) < 1:
        parser.error("incorrect number of arguments")
    return options,args

def run(asmb_fn):
    asmb_input=IMP.multifit.read_settings(asmb_fn)
    asmb_input.set_was_used(True)
    for i in range(asmb_input.get_number_of_component_headers()):
        mol_fn=asmb_input.get_component_header(i).get_filename()
        mol_surface_fn=asmb_input.get_component_header(i).get_surface_fn()
        m = IMP.Model()
        h = IMP.atom.read_pdb(mol_fn, m,
                              IMP.atom.NonWaterNonHydrogenPDBSelector())
        IMP.multifit.write_connolly_surface(IMP.atom.get_leaves(h),
                                            mol_surface_fn, 10.0, 1.8)

def main():
    options,args = parse_args()
    asmb_input = args[0]
    run(asmb_input)

if __name__=="__main__":
    main()
