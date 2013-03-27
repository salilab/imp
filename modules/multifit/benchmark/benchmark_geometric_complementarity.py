#!/usr/bin/env python
#
# This benchmark tries to compute the scores for the first 10 entries in the
# benchmark file using ComplementarityRestraint.
# Output:
# ComplementarityRestraint, (time in sec/score computation), (percentage error)
#
# Copyright 2007-2013 IMP Inventors. All rights reserved.
#

import sys
import IMP
import IMP.core
import IMP.atom
import IMP.algebra
import IMP.benchmark
import IMP.multifit
import time


def read_transformations():
    res = list()
    inf = open(IMP.benchmark.get_data_path("1j1A_docking.imp.txt"), "r")
    inf.readline() # header
    for line in inf:
        fields = line.strip().split('|')
        rots = [float(x) for x in fields[2].split()]
        trans = [float(x) for x in fields[3].split()]
        score = float(fields[7].strip())
        res.append((IMP.algebra.Transformation3D(
          IMP.algebra.Rotation3D(rots[0], rots[1], rots[2], rots[3]),
          IMP.algebra.Vector3D(trans[0], trans[1], trans[2])), score))
    inf.close()
    return res


def load_proteins(model):
    pdb_name = IMP.benchmark.get_data_path("1j1jA.pdb")
    h1 = IMP.atom.read_pdb(pdb_name, model)
    mh1 = IMP.atom.get_by_type(h1, IMP.atom.ATOM_TYPE)
    r1 = [x.get_particle() for x in mh1]
    rb1 = IMP.atom.setup_as_rigid_body(h1)
    h2 = IMP.atom.read_pdb(pdb_name, model)
    mh2 = IMP.atom.get_by_type(h2, IMP.atom.ATOM_TYPE)
    r2 = [x.get_particle() for x in mh2]
    rb2 = IMP.atom.setup_as_rigid_body(h2)
    return (r1, r2, rb1, rb2)


def perform_benchmark(model, tr_list, r1, r2, rb1, rb2, maxiter):
    rest = IMP.multifit.ComplementarityRestraint(r1, r2)
    rest.set_complementarity_value(-1)
    rest.set_complementarity_thickness(1)
    rest.set_interior_layer_thickness(1)
    model.add_restraint(rest)
    maxiter = min(maxiter, len(tr_list))
    for i in xrange(maxiter):
        start = time.time()
        IMP.core.transform(rb2, tr_list[i][0])
        score = 8*rest.evaluate(False)
        orig_score = tr_list[i][1]
        pct_error = 100*abs((score - orig_score)/orig_score)
        IMP.core.transform(rb2, tr_list[i][0].get_inverse())
        elapsed = time.time() - start
        IMP.benchmark.report("ComplementarityRestraint %d" % i, "",
                             elapsed, pct_error)


IMP.base.set_log_level(IMP.base.SILENT)
tr_list = read_transformations()
model = IMP.Model()
r1, r2, rb1, rb2 = load_proteins(model)
perform_benchmark(model, tr_list, r1, r2, rb1, rb2, 4)
