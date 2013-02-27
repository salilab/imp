#!/usr/bin/env python

__doc__ = "Generate complete models from Chimera transformations file."

import sys
import IMP
import IMP.atom
from optparse import OptionParser

def parse_args():
    usage = """%prog [options] <subunit> <symmetry degree>
          <transformations file> <number of models> <output models>

This script, given the structure of a single subunit and the Chimera
transformations file, applies the transformations to generate a number of
complete models.

  <subunit>               subunit PDB, the same given to MultiFit.
  <symmetry degree>       Cn degree.
  <transformations file>  MultiFit output in chimera output format.
  <number of models>      number of models to print.
  <output models>         Solutions are written as output.i.pdb."""
    parser = OptionParser(usage)
    opts, args = parser.parse_args()
    if len(args) != 5:
        parser.error("incorrect number of arguments")
    return args

def get_transformations(sol):
    [ind,dock_rot_s,dock_trans_s,fit_rot_s,fit_trans_s,cc_s]=sol.split("|")
    s=dock_rot_s.split(" ")
    s1=dock_trans_s.split(" ")
    dock_t=IMP.algebra.Transformation3D(
        IMP.algebra.get_rotation_from_matrix(
        float(s[0]),float(s[1]),float(s[2]),
        float(s[3]),float(s[4]),float(s[5]),
        float(s[6]),float(s[7]),float(s[8])),
        IMP.algebra.Vector3D(float(s1[0]),float(s1[1]),float(s1[2])))
    s=fit_rot_s.split(" ")
    s1=fit_trans_s.split(" ")
    fit_t=IMP.algebra.Transformation3D(
        IMP.algebra.get_rotation_from_matrix(
        float(s[0]),float(s[1]),float(s[2]),
        float(s[3]),float(s[4]),float(s[5]),
        float(s[6]),float(s[7]),float(s[8])),
        IMP.algebra.Vector3D(float(s1[0]),float(s1[1]),float(s1[2])))
    return dock_t,fit_t

def run(subunit_fn,symm_deg,sol_fn,num,output_fn):
    mdl=IMP.Model()
    mhs=[]
    for i in range(symm_deg):
        mh=IMP.atom.read_pdb(subunit_fn,mdl)
        mhs.append(mh)
        IMP.atom.setup_as_rigid_body(mh)
    lines=open(sol_fn).readlines()
    for j,sol in enumerate(lines[:num]):
        dock_t,fit_t=get_transformations(sol)
        curr_t=dock_t
        for i in range(symm_deg):
            IMP.core.transform(IMP.core.RigidBody(mhs[i]),curr_t)
            curr_t=dock_t*curr_t
        for i in range(symm_deg):
            IMP.core.transform(IMP.core.RigidBody(mhs[i]),fit_t)
        IMP.atom.write_pdb(mhs,output_fn+".%03d.pdb" % j)

def main():
    args = parse_args()
    run(args[0],int(args[1]),args[2],int(args[3]),args[4])

if __name__ == "__main__":
    main()
