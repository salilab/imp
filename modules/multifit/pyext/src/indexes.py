#!/usr/bin/env python

from __future__ import print_function
import IMP.multifit
from IMP import ArgumentParser

__doc__ = "Generate indexes of fitting solutions."

def parse_args():
    desc = """Generate indexes of fitting solutions."""
    p = ArgumentParser(description=desc)
    p.add_argument("assembly_name", help="name of the assembly")
    p.add_argument("assembly_file", help="assembly file name")
    p.add_argument("num_fits", type=int, help="number of fits")
    p.add_argument("indexes_file", help="indexes file name")
    return p.parse_args()


def run(assembly_name, asmb_fn, num_fits, mapping_fn=""):
    IMP.set_log_level(IMP.WARNING)
    asmb_input = IMP.multifit.read_settings(asmb_fn)
    asmb_input.set_was_used(True)
    ap_em_fn = asmb_input.get_assembly_header().get_coarse_over_sampled_ap_fn()
    if mapping_fn == "":
        mapping_fn = assembly_name + ".indexes.mapping.input"
    mapping_data = open(mapping_fn, "w")
    mapping_data.write("|anchors|" + ap_em_fn + "|\n")
    for i in range(asmb_input.get_number_of_component_headers()):
        name = asmb_input.get_component_header(i).get_name()
        trans_fn = asmb_input.get_component_header(i).get_transformations_fn()
        fits = IMP.multifit.read_fitting_solutions(trans_fn)
        print("number of fits for component", i, "is", len(fits), trans_fn)
        index_fn = assembly_name + "." + name + ".fit.indexes.txt"
        indexes = []
        for i in range(min(num_fits, len(fits))):
            indexes.append([i])
        IMP.multifit.write_paths(indexes, index_fn)
        mapping_data.write("|protein|" + name +
                           "|" + index_fn + "|\n")
    mapping_data.close()


def main():
    args = parse_args()
    run(args.assembly_name, args.assembly_file, args.num_fits,
        args.indexes_file)

if __name__ == "__main__":
    main()
