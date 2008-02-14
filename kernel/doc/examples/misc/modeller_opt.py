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

# the order here is a little tricky and can't be messed with
# the new energy term must be added to env before the Modeller model is created
imp_model = IMP.Model()
particles = []

t = env.edat.energy_terms
t.append(IMP.modeller_intf.IMPRestraints(imp_model, particles))

# get particles for Modeller
print "adding particles"
modeller_model = modeller.model(env, file='./particles.pdb')
atoms = modeller_model.atoms
# intialize particles for IMP
IMP.modeller_intf.init_imp_from_modeller(imp_model, particles, atoms)

# create a restraint set
print "adding IMP restraints"
rs = IMP.RestraintSet("dist_rsrs")
imp_model.add_restraint(rs)

# create IMP restraints
# the restraint pointer must be kept alive in Python or it gets deleted in IMP
score_func = IMP.Harmonic()
dist_rsr = []
dist_rsr.append(IMP.DistanceRestraint(imp_model, particles[0], particles[2], 5.0, 0.1, score_func))
dist_rsr.append(IMP.DistanceRestraint(imp_model, particles[1], particles[2], 5.0, 0.1, score_func))
for i in range(0,len(dist_rsr)):
    rs.add_restraint(dist_rsr[i])

atmsel = modeller.selection(modeller_model)
print atmsel.energy()

# create a modeller optimizer and optimize the model using the IMP restraints
opt = modeller.optimizers.conjugate_gradients()
new_mdl = opt.optimize (atmsel, max_iterations=55, actions=None)
print atmsel.energy()

# show the results
IMP.modeller_intf.show_modeller_and_imp(atoms, particles)
IMP.utils.show_distances(particles)
IMP.utils.show_particles_pos(particles)
for p in particles:
    print 'particle'+str( p.get_index().get_index())
    IMP.XYZDecorator.cast(p).show()
