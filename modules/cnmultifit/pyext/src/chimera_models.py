#!/usr/bin/env python

__doc__ = "Generate complete models from Chimera transformations file."

import sys
import IMP
import IMP.atom
from IMP import ArgumentParser


def parse_args():
    desc = """
This script, given the structure of a single subunit and the Chimera
transformations file, applies the transformations to generate a number of
complete models."""

    p = ArgumentParser(description=desc)
    p.add_argument('subunit', help="subunit PDB, the same given to MultiFit")
    p.add_argument('degree', type=int, help="Cn symmetry degree")
    p.add_argument('transform_file',
                   help="MultiFit output file in Chimera output format")
    p.add_argument('num_models', type=int,
                   help="number of output models")
    p.add_argument('output',
                   help="solution filename prefix; solutions are written "
                        "as <output>.i.pdb")
    return p.parse_args()


def get_transformations(sol):
    [ind, dock_rot_s, dock_trans_s, fit_rot_s,
        fit_trans_s, cc_s] = sol.split("|")
    s = dock_rot_s.split(" ")
    s1 = dock_trans_s.split(" ")
    dock_t = IMP.algebra.Transformation3D(
        IMP.algebra.get_rotation_from_matrix(
            float(s[0]), float(s[1]), float(s[2]),
            float(s[3]), float(s[4]), float(s[5]),
            float(s[6]), float(s[7]), float(s[8])),
        IMP.algebra.Vector3D(float(s1[0]), float(s1[1]), float(s1[2])))
    s = fit_rot_s.split(" ")
    s1 = fit_trans_s.split(" ")
    fit_t = IMP.algebra.Transformation3D(
        IMP.algebra.get_rotation_from_matrix(
            float(s[0]), float(s[1]), float(s[2]),
            float(s[3]), float(s[4]), float(s[5]),
            float(s[6]), float(s[7]), float(s[8])),
        IMP.algebra.Vector3D(float(s1[0]), float(s1[1]), float(s1[2])))
    return dock_t, fit_t


def run(subunit_fn, symm_deg, sol_fn, num, output_fn):
    mdl = IMP.Model()
    mhs = []
    rbs = []
    for i in range(symm_deg):
        mh = IMP.atom.read_pdb(subunit_fn, mdl)
        mhs.append(mh)
        rbs.append(IMP.atom.create_rigid_body(mh))
    lines = open(sol_fn).readlines()
    for j, sol in enumerate(lines[:num]):
        dock_t, fit_t = get_transformations(sol)
        curr_t = dock_t
        for i in range(symm_deg):
            IMP.core.transform(rbs[i], curr_t)
            curr_t = dock_t * curr_t
        for i in range(symm_deg):
            IMP.core.transform(rbs[i], fit_t)
        IMP.atom.write_pdb(mhs, output_fn + ".%03d.pdb" % j)


def main():
    args = parse_args()
    run(args.subunit, args.degree, args.transform_file, args.num_models,
        args.output)

if __name__ == "__main__":
    main()
