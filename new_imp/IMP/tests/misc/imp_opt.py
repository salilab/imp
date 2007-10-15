import modeller
import modeller.optimizers
import IMP
import IMP.modeller_intf
import IMP.utils

# intialize Modeller
modeller.log.level(0,0,0,0,0)
env = modeller.environ()
env.io.atom_files_directory = '../data/'
env.edat.dynamic_sphere = False
env.libs.topology.read(file='$(LIB)/top_heav.lib')
env.libs.parameters.read(file='$(LIB)/par.lib')

# get particles for Modeller
modeller_model = modeller.model(env, file='./particles.pdb')
atoms = modeller_model.atoms

# intialize particles for IMP
imp_model = IMP.Model()
print "adding particles"
particles = []
IMP.modeller_intf.init_imp_from_modeller(imp_model, particles, atoms)

# create a restraint set
rs = IMP.RestraintSet("dist_rsrs")
imp_model.add_restraint(rs)

# add Modeller restraints
rsr = modeller_model.restraints
rsr.add(modeller.forms.gaussian(group=modeller.physical.xy_distance,
                          feature=modeller.features.distance(atoms[1], atoms[2]),
                          mean=5., stdev=0.1))

# make Modeller restraints accessible to IMP
r = IMP.modeller_intf.ModellerRestraints(imp_model, modeller_model, particles)
print "adding Modeller restraints"
rs.add_restraint(r)

# create IMP restraints
score_func = IMP.Harmonic()
dist_rsr = IMP.DistanceRestraint(imp_model, particles[0], particles[2], 5.0, 0.1, score_func)
print "adding IMP restraints"
rs.add_restraint(dist_rsr)

IMP.modeller_intf.show_modeller_and_imp(atoms, particles)

print "run optimizer"
steepest_descent = IMP.SteepestDescent()
steepest_descent.optimize(imp_model, 10)

IMP.modeller_intf.show_modeller_and_imp(atoms, particles)
IMP.utils.show_distances(particles)
