import modeller
import modeller.optimizers
import IMP
import IMP.modeller_intf
import IMP.test

# intialize Modeller
modeller.log.level(0,0,0,0,0)
env = modeller.environ()
env.io.atom_files_directory = '../data/'
env.edat.dynamic_sphere = False
env.libs.topology.read(file='$(LIB)/top_heav.lib')
env.libs.parameters.read(file='$(LIB)/par.lib')

# get particles for Modeller
mdl = modeller.model(env, file='./particles.pdb')
atoms = mdl.atoms

# intialize particles for IMP
model = IMP.Model()
print "adding particles"
particles = []
IMP.modeller_intf.Init_IMP_From_Modeller(model, particles, atoms)

# create a restraint set
rs = IMP.RestraintSet("dist_rsrs")
model.add_restraint_set(rs)

# add Modeller restraints
rsr = mdl.restraints
rsr.add(modeller.forms.gaussian(group=modeller.physical.xy_distance,
                          feature=modeller.features.distance(atoms[1], atoms[2]),
                          mean=5., stdev=0.1))

# rsr.add(modeller.forms.gaussian(group=modeller.physical.xy_distance,
#                          feature=modeller.features.distance(atoms[1], atoms[2]),
#                          mean=3., stdev=0.1))

# Replace the last restraint with an IMP restraint
# rsr.add(modeller.forms.gaussian(group=modeller.physical.xy_distance,
#                          feature=modeller.features.distance(atoms[0], atoms[2]),
#                          mean=3., stdev=0.1))

# make Modeller restraints accessible to IMP
r = IMP.modeller_intf.Modeller_Restraints(model, mdl, particles)
print "adding Modeller restraints"
rs.add_restraint(r)

# create IMP restraints
score_func = IMP.Harmonic()
dist_rsr = IMP.DistanceRestraint(model, particles[0], particles[2], 5.0, 0.1, score_func)
print "adding IMP restraints"
rs.add_restraint(dist_rsr)

IMP.modeller_intf.Show_Modeller_and_IMP(atoms, particles)

print "run optimizer"
steepest_descent = IMP.SteepestDescent()
steepest_descent.optimize(model, 10, 0.0)

IMP.modeller_intf.Show_Modeller_and_IMP(atoms, particles)

IMP.modeller_intf.Show_Distances(particles)


particles = IMP.ParticleIterator()
particles.reset(model)
while particles.next():
    particle = particles.get()
    print particle

restraint_sets = IMP.RestraintSet_Iterator()
restraint_sets.reset(model)
while restraint_sets.next():
    restraint_set = restraint_sets.get()
    print restraint_set.name()
