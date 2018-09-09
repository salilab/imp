#!/usr/bin/env python
import sys
import os
import string
import IMP.multifit
try:
    from argparse import ArgumentParser
except ImportError:
    from IMP._compat_argparse import ArgumentParser


def usage():
    desc = "A script for getting fits with best RMSD to the reference"
    parser = ArgumentParser(description=desc)
    parser.add_argument("-d", action="store_true", dest="use_dock",
                        help="if set the docking transformation is used "
                             "(and not the fitting transformation)")
    parser.add_argument("-m", dest="dir", default="./",
                        help="output models directory")
    parser.add_argument("mol_file", help="molecule file name")
    parser.add_argument("transformations", help="transformations file name")
    parser.add_argument("start", nargs='?', type=int, default=0,
                        help="first transformation index (default is first)")
    parser.add_argument("end", nargs='?', type=int, default=99999999,
                        help="last transformation index (default is last)")
    return parser.parse_args()

def run(mol_fn, trans_fn, first, last, dock_trans, out_dir):
    IMP.set_log_level(IMP.SILENT)
    mdl = IMP.Model()
    print mol_fn
    mh = IMP.atom.read_pdb(mol_fn, mdl)
    xyz = IMP.core.XYZs(IMP.core.get_leaves(mh))
    IMP.atom.create_rigid_body(mh)
    rb = IMP.core.RigidMember(xyz[0]).get_rigid_body()
    name = mol_fn.split("/")[-1].split(".pdb")[0]
    print name, trans_fn
    fits = IMP.multifit.read_fitting_solutions(trans_fn)
    print "=======2"
    for i, rec in enumerate(fits[first:last]):
        fit_t = rec.get_fit_transformation()
        if dock_trans:
            fit_t = rec.get_dock_transformation()
        IMP.core.transform(rb, fit_t)
        IMP.atom.write_pdb(mh, out_dir + name + "." + str(i) + ".pdb")
        IMP.core.transform(rb, fit_t.get_inverse())
    print "=======3"

if __name__ == "__main__":
    args = usage()
    run(args.mol_file, args.transformations, first, last, args.use_dock,
        args.dir)
