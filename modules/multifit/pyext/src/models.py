#!/usr/bin/env python

from __future__ import print_function
import IMP.multifit
from IMP import ArgumentParser

__doc__ = "Write output models as PDB files."

# analyse the ensemble, first we will do the rmsd stuff

def parse_args():
    desc = """Write output models."""
    p = ArgumentParser(description=desc)
    p.add_argument("-m", "--max", type=int, dest="max", default=None,
                   help="maximum number of models to write")
    p.add_argument("assembly_file", help="assembly file name")
    p.add_argument("proteomics_file", help="proteomics file name")
    p.add_argument("mapping_file", help="mapping file name")
    p.add_argument("combinations_file", help="combinations file name")
    p.add_argument("model_prefix", help="model output file name prefix")
    return p.parse_args()


def run(asmb_fn, proteomics_fn, mapping_fn, combs_fn, model_output, max_comb):
    # get rmsd for subunits
    mdl = IMP.Model()
    combs = IMP.multifit.read_paths(combs_fn)
    sd = IMP.multifit.read_settings(asmb_fn)
    sd.set_was_used(True)
    prot_data = IMP.multifit.read_proteomics_data(proteomics_fn)
    mapping_data = IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                             mapping_fn)
    ensmb = IMP.multifit.load_ensemble(sd, mdl, mapping_data)
    mhs = ensmb.get_molecules()
    print("number of combinations:", len(combs), max_comb)
    for i, comb in enumerate(combs[:max_comb]):
        if i % 500 == 0:
            print(i)
        ensmb.load_combination(comb)
        print(model_output + "." + str(i) + ".pdb")
        IMP.atom.write_pdb(mhs, model_output + "." + str(i) + ".pdb")
        ensmb.unload_combination(comb)


def main():
    args = parse_args()
    run(args.assembly_file, args.proteomics_file, args.mapping_file,
        args.combinations_file, args.model_prefix, args.max)

if __name__ == "__main__":
    main()
