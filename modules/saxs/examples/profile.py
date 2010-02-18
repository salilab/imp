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
saxs_score = IMP.saxs.Score(exp_profile)
chi = saxs_score.compute_chi_score(model_profile)
print 'Chi = ' + str(chi)
