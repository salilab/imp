## \example saxs/profile_fit.py
## In this example, we read a protein from a PDB file and experimental profile file. Next we compute the theoretical profile from the PDB file and fit it to the experimental one. Unlike in profile.py example, here we fit the profile with adjustment of the excluded volume and hydration layer density of the molecule.
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
                      IMP.atom.NonWaterNonHydrogenPDBSelector(), True, True)

#! read experimental profile
exp_profile = IMP.saxs.Profile(IMP.saxs.get_example_path('lyzexp.dat'))

print 'min_q = ' + str(exp_profile.get_min_q())
print 'max_q = ' + str(exp_profile.get_max_q())
print 'delta_q = ' + str(exp_profile.get_delta_q())

#! select particles from the model
particles = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)

#! add radius for water layer computation
ft = IMP.saxs.default_form_factor_table()
for i in range(0, len(particles)):
    radius = ft.get_radius(particles[i])
    IMP.core.XYZR.setup_particle(particles[i], radius)
# compute surface accessibility
s = IMP.saxs.SolventAccessibleSurface()
surface_area = s.get_solvent_accessibility(IMP.core.XYZRs(particles))

#! calculate SAXS profile
delta_q = 0.5 / 500
model_profile = IMP.saxs.Profile(0.0, 0.5, delta_q)
model_profile.calculate_profile_partial(particles, surface_area)
#model_profile.write_SAXS_file('6lyz.dat')

#! calculate chi score (should be ~0.5 for this example)
saxs_score = IMP.saxs.ProfileFitterChi(exp_profile)
chi = saxs_score.compute_score(model_profile)
print 'Chi without parameter fitting = ' + str(chi)

chi = (saxs_score.fit_profile(model_profile)).get_chi()
print 'Chi after adjustment of excluded volume and water layer parameters = ' + str(chi)
