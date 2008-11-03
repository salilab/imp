import IMP
import IMP.core
import random

# This example constructs a set of particles which are restrained
# to form a chain via bonds between successive particles. In addition
# the head and the tail of the chain are restrained to be close to one
# another.

#IMP.set_log_level(IMP.VERBOSE)
np=20
radius =1.0
rk= IMP.FloatKey("radius")
m= IMP.Model()
# The particles in the chain
chain= IMP.core.ListParticleContainer()
for i in range(0,np):
    p= IMP.Particle()
    pi= m.add_particle(p)
    d= IMP.core.XYZDecorator.create(p)
    d.set_coordinates(IMP.random_vector_in_box(IMP.Vector3D(0,0,0),
                                               IMP.Vector3D(10,10,10)))
    d.set_coordinates_are_optimized(True)
    p.add_attribute(rk, radius, False)
    chain.add_particle(p)

# create a bond between successive particles
IMP.core.BondedDecorator.create(chain.get_particle(0))
bonds= IMP.core.ListParticleContainer()
for i in range(1, chain.get_number_of_particles()):
    bp= IMP.core.BondedDecorator.cast(chain.get_particle(i-1))
    bpr= IMP.core.BondedDecorator.create(chain.get_particle(i))
    b= IMP.core.custom_bond(bp, bpr, 1.5*radius, 10)gd
    bonds.add_particle(b.get_particle())

# If you want to inspect the particles
# Notice that each bond is a particle
for p in m.get_particles():
    p.show()

# Set up the nonbonded list
nbl= IMP.core.ClosePairsScoreState(chain)
nbli= m.add_score_state(nbl)
# Exclude bonds from closest pairs
fl= nbl.get_close_pairs_container()
fl.add_particle_pair_container(IMP.core.BondDecoratorParticlePairContainer())

# Set up excluded volume
ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1), rk)
evr= IMP.core.ParticlePairsRestraint(ps, fl)
evri= m.add_restraint(evr)

# Restraint for bonds
bss= IMP.core.BondDecoratorSingletonScore(IMP.core.Harmonic(0,1))
br= IMP.core.ParticlesRestraint(bss, bonds)
bri= m.add_restraint(br)

# Tie the ends of the chain
# We cound have used a bond instead
p= IMP.ParticlePair(chain.get_particle(0), chain.get_particle(chain.get_number_of_particles()-1))
pps= IMP.ParticlePairs()
pps.append(p)
cr= IMP.core.PairListRestraint(
           IMP.core.SphereDistancePairScore(IMP.core.Harmonic(3,1), rk), pps)
cri=m.add_restraint(cr)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

# Write the progression of states as the system is optimized to
# the files state.000.vrml, state.001.vrml etc.
vrml= IMP.core.VRMLLogOptimizerState(chain, "state.%03d.vrml")
vrml.set_radius_key(rk)
vrml.update()
vrml.set_skip_steps(100)
o.add_optimizer_state(vrml)

# We probably don't need this many steps
o.optimize(1000)

# Write the final state out
vrml.write_next_file()
