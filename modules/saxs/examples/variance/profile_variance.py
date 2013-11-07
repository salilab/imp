# \example saxs/profile.py
## In this example, we read a protein from a PDB file and experimental profile file. Next we compute the theoretical profile from the PDB file and fit it to the experimental one.
##
## This application is available as a web service at salilab.org/foxs. It is also available as C++ code in IMP/applications.
##
## The experimental data for lysozyme is taken from crysol program (www.embl-hamburg.de/ExternalInfo/Research/Sax/crysol.html)
##

import IMP
import IMP.atom
import IMP.core
import IMP.saxs
import os
import sys
import numpy

m = IMP.Model()

calc_variance=True

#! read PDB
mp= IMP.atom.read_pdb(IMP.saxs.get_example_path('6lyz.pdb'), m,
                      IMP.atom.CAlphaPDBSelector())

#! select particles from the model
particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
coords = array([list(IMP.core.XYZ(p).get_coordinates()) for p in particles])

#! calculate SAXS profile
model_profile = IMP.saxs.Profile()
model_profile.calculate_profile(particles,IMP.saxs.CA_ATOMS,False,calc_variance)

fl=open('6lyz_profile.dat','w')
for q in xrange(model_profile.size()):
    fl.write('%G ' % q)
    fl.write('%G ' % model_profile.get_q(q))
    if calc_variance:
        fl.write('%G ' % model_profile.get_intensity(q))
        fl.write('%G\n' % numpy.sqrt(model_profile.get_variance(q,q)))
    else:
        fl.write('%G\n' % model_profile.get_intensity(q))

if calc_variance:
    fl=open('6lyz_var.dat','w')
    for q1 in xrange(model_profile.size()):
        for q2 in xrange(model_profile.size()):
            fl.write('%G ' % model_profile.get_q(q1))
            fl.write('%G ' % model_profile.get_q(q2))
            fl.write('%G\n' % model_profile.get_variance(q1,q2))
        fl.write('\n')

    fl=open('6lyz_rel.dat','w')
    for q1 in xrange(model_profile.size()):
        for q2 in xrange(model_profile.size()):
            I1 = model_profile.get_intensity(q1)
            I2 = model_profile.get_intensity(q2)
            fl.write('%G ' % model_profile.get_q(q1))
            fl.write('%G ' % model_profile.get_q(q2))
            fl.write('%G\n' % (model_profile.get_variance(q1,q2)/(I1*I2)))
        fl.write('\n')
