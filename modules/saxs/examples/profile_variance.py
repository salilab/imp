import IMP
import IMP.atom
import IMP.core
import IMP.saxs
import os,sys
from math import log,sqrt

tau = 0.1

m = IMP.Model()

#! read PDB
mp= IMP.atom.read_pdb(IMP.saxs.get_example_path('6lyz.pdb'), m,
                      IMP.atom.NonWaterNonHydrogenPDBSelector())

#! read experimental profile
exp_profile = IMP.saxs.Profile(IMP.saxs.get_example_path('lyzexp.dat'))

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
for i in xrange(model_profile.size()):
    qi = model_profile.get_q(i)
    ii = model_profile.get_intensity(i)
    for j in xrange(model_profile.size()):
        qj = model_profile.get_q(j)
        ij = model_profile.get_intensity(j)
        vij = model_profile.get_variance(i,j)/(ii*ij)
        fl.write('%f %f %f\n' % (qi,qj,vij))
    fl.write('\n')

fl=open('6lyz_%.1f.dat' % tau,'w')
for i in xrange(model_profile.size()):
    ii = model_profile.get_intensity(i)
    qi = model_profile.get_q(i)
    vi = sqrt(model_profile.get_variance(i,i)/(ii*ii))
    fl.write("%f %f %f\n" % (qi,log(ii),vi))
