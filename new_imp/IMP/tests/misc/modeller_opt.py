import modeller
import modeller.optimizers
import sys
sys.path.append("../../")
sys.path.append("../python_libs/")
import imp2
import IMP_Modeller_Intf

# intialize Modeller
modeller.log.level(0,0,0,0,0)
env = modeller.environ()
env.io.atom_files_directory = '../data/'
env.edat.dynamic_sphere = False
env.libs.topology.read(file='$(LIB)/top_heav.lib')
env.libs.parameters.read(file='$(LIB)/par.lib')

# the order here is a little tricky and can't be messed with
# the new energy term must be added to env before the Modeller model is created
imp_model = imp2.Model()
particles = []

t = env.edat.energy_terms
t.append(IMP_Modeller_Intf.IMP_Restraints(imp_model, particles))

# get particles for Modeller
print "adding particles"
modeller_model = modeller.model(env, file='./particles.pdb')
atoms = modeller_model.atoms
# intialize particles for IMP
IMP_Modeller_Intf.Init_IMP_From_Modeller(imp_model, particles, atoms)

# create a restraint set
print "adding IMP restraints"
rs = imp2.Restraint_Set("dist_rsrs")
imp_model.add_restraint_set(rs)

# create IMP restraints
# the restraint pointer must be kept alive in Python or it gets deleted in IMP
score_func = imp2.Harmonic()
dist_rsr = []
dist_rsr.append(imp2.RSR_Distance(imp_model, particles[0], particles[2], 5.0, 0.1, score_func))
dist_rsr.append(imp2.RSR_Distance(imp_model, particles[1], particles[2], 5.0, 0.1, score_func))
for i in range(0,len(dist_rsr)):
    rs.add_restraint(dist_rsr[i])

atmsel = modeller.selection(modeller_model)
print atmsel.energy()

# create a modeller optimizer and optimize the model using the IMP restraints
opt = modeller.optimizers.conjugate_gradients()
new_mdl = opt.optimize (atmsel, max_iterations=55, actions=None)
print atmsel.energy()

# show the results
IMP_Modeller_Intf.Show_Modeller_and_IMP(atoms, particles)
IMP_Modeller_Intf.Show_Distances(particles)
IMP_Modeller_Intf.Show_IMP_Particles_Pos(particles)
