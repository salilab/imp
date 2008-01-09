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

# intialize particles for IMP
model = IMP.Model()
particles = []

# add IMP model and restraints as an energy term to Modeller model
t = env.edat.energy_terms
t.append(IMP.modeller_intf.IMPRestraints(model, particles))

# get particles for Modeller
mdl = IMP.modeller_intf.create_particles(12, env, model, particles)

p1 = particles[0]
p1.add_float("radius", 2.0, False)
p1.add_int("protein", 1)
p1.add_int("id", 1)

p1 = particles[1]
p1.add_float("radius", 2.0, False)
p1.add_int("protein", 1)
p1.add_int("id", 2)

p1 = particles[2]
p1.add_float("radius", 2.0, False)
p1.add_int("protein", 1)
p1.add_int("id", 3)

p1 = particles[3]
p1.add_float("radius", 1.5, False)
p1.add_int("protein", 2)
p1.add_int("id", 4)

p1 = particles[4]
p1.add_float("radius", 1.5, False)
p1.add_int("protein", 2)
p1.add_int("id", 5)

p1 = particles[5]
p1.add_float("radius", 1.5, False)
p1.add_int("protein", 2)
p1.add_int("id", 6)

p1 = particles[6]
p1.add_float("radius", 1.5, False)
p1.add_int("protein", 2)
p1.add_int("id", 7)

p1 = particles[7]
p1.add_float("radius", 1.0, False)
p1.add_int("protein", 3)
p1.add_int("id", 8)

p1 = particles[8]
p1.add_float("radius", 1.0, False)
p1.add_int("protein", 3)
p1.add_int("id", 9)

p1 = particles[9]
p1.add_float("radius", 1.0, False)
p1.add_int("protein", 3)
p1.add_int("id", 10)

p1 = particles[10]
p1.add_float("radius", 1.0, False)
p1.add_int("protein", 3)
p1.add_int("id", 11)

p1 = particles[11]
p1.add_float("radius", 1.0, False)
p1.add_int("protein", 3)
p1.add_int("id", 12)

atmsel = modeller.selection(mdl)
atoms = mdl.atoms
atmsel.randomize_xyz(deviation=100.0)

rs = IMP.RestraintSet("connect")
model.add_restraint(rs)
score_func = IMP.Harmonic()

# add connectivity restraints

particle_indexes = IMP.Ints()
rsrs = []

# connect 3 proteins together
particle_indexes.clear()
for i in range(0, 12):
    particle_indexes.push_back(i)
rsrs.append(IMP.ConnectivityRestraint(model, particle_indexes, "protein", "radius", 0.1, score_func))

# connect particles in protein1 together
particle_indexes.clear()
for i in range(0, 3):
    particle_indexes.push_back(i)
rsrs.append(IMP.ConnectivityRestraint(model, particle_indexes, "id", "radius", 0.1, score_func))

# connect particles in protein2 together
particle_indexes.clear()
for i in range(3, 7):
    particle_indexes.push_back(i)
rsrs.append(IMP.ConnectivityRestraint(model, particle_indexes, "id", "radius", 0.1, score_func))

# connect particles in protein3together
particle_indexes.clear()
for i in range(7, 12):
    particle_indexes.push_back(i)
rsrs.append(IMP.ConnectivityRestraint(model, particle_indexes, "id", "radius", 0.1, score_func))

# add restraints
for i in range(0, len(rsrs)):
    rs.add_restraint(rsrs[i])

model.set_up_trajectory("trajectory.txt")
opt = modeller.optimizers.conjugate_gradients()
new_mdl = opt.optimize (atmsel, max_iterations=55, actions=None)
print atmsel.energy()

mdl.write (file='out.pdb', model_format='PDB')

IMP.utils.show_distances(particles)
IMP.utils.show_particles(particles)
