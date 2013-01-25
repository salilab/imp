## \example saxs/profile.py
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


m = IMP.Model()

#! read PDB
mp= IMP.atom.read_pdb(IMP.saxs.get_example_path('6lyz.pdb'), m,
                      IMP.atom.NonWaterNonHydrogenPDBSelector())

#! read experimental profile
exp_profile = IMP.saxs.Profile(IMP.saxs.get_example_path('lyzexp.dat'))

print 'min_q = ' + str(exp_profile.get_min_q())
print 'max_q = ' + str(exp_profile.get_max_q())
print 'delta_q = ' + str(exp_profile.get_delta_q())

#! select particles from the model
particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

#! calculate SAXS profile
model_profile = IMP.saxs.Profile()
model_profile.calculate_profile(particles)
model_profile.write_SAXS_file('6lyz.dat')

#! calculate chi score (should be ~0.5 for this example)
saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
chi = saxs_score.compute_score(model_profile)
print 'Chi = ' + str(chi)


#! convert to p(r)
pr = IMP.saxs.RadialDistributionFunction(0.5)
model_profile.profile_2_distribution(pr, 48.0)
pr.normalize()
pr.show()
