#!/usr/bin/env python
import IMP
import IMP.atom
import IMP.core
import IMP.saxs
import os,sys
from numpy import *

tau = .1

m = IMP.Model()

#! read PDB
mp= IMP.atom.read_pdb('6lyz.pdb', m,
                      IMP.atom.NonWaterNonHydrogenPDBSelector())

#! read experimental profile
exp_profile = IMP.saxs.Profile('lyzexp_cut.dat')

qmin = exp_profile.get_min_q()
qmax = exp_profile.get_max_q()
dq = exp_profile.get_delta_q()

#! select particles from the model
particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

#! calculate SAXS profile
model_profile = IMP.saxs.Profile(qmin,qmax,dq)
model_profile.calculate_profile(particles, IMP.saxs.HEAVY_ATOMS,
        False, True, tau)

fl=open('matrix_%.1f' % tau, 'w')
mat = []
for i in xrange(model_profile.size()):
    qi = model_profile.get_q(i)
    ii = model_profile.get_intensity(i)
    tmp=[]
    for j in xrange(model_profile.size()):
        qj = model_profile.get_q(j)
        ij = model_profile.get_intensity(j)
        vij = model_profile.get_variance(i,j)
        tmp.append(vij)
        #tmp.append(vij)
        fl.write('%s %s %s\n' % (qi,qj,vij))
    mat.append(tmp)
    fl.write('\n')
mat = array(mat)
fl=open('eigenvals','w')
for i in linalg.eigvalsh(mat):
    fl.write('%s\n' % i)

fl=open('6lyz_%.1f.dat' % tau,'w')
for i in xrange(model_profile.size()):
    ii = model_profile.get_intensity(i)
    qi = model_profile.get_q(i)
    vi = sqrt(model_profile.get_variance(i,i))
    fl.write("%f %f %f\n" % (qi,ii,vi))
