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
chain= IMP.Particles()
for i in range(0,np):
    p= IMP.Particle()
    pi= m.add_particle(p)
    d= IMP.core.XYZDecorator.create(p)
    d.set_coordinates(IMP.random_vector_in_box(IMP.Vector3D(0,0,0),
                                               IMP.Vector3D(10,10,10)))
    d.set_coordinates_are_optimized(True)
    p.add_attribute(rk, radius, False)
    chain.append(p)

# create a bond between successive particles
IMP.core.BondedDecorator.create(chain[0])
for i in range(1, len(chain)):
    bp= IMP.core.BondedDecorator.cast(chain[i-1])
    bpr= IMP.core.BondedDecorator.create(chain[i])
    b= IMP.core.custom_bond(bp, bpr, 1.5*radius, 10)

# If you want to inspect the particles
# Notice that each bond is a particle
for p in m.get_particles():
    p.show()

# Set up the nonbonded list
nbl= IMP.core.AllNonbondedListScoreState(1, rk, chain)
nbli= m.add_score_state(nbl)
# This ScoreState uses the bonds constructed above to restrain
bl= IMP.core.BondDecoratorListScoreState(chain)
bli= nbl.add_bonded_list(bl)

# Set up excluded volume
ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1), rk)
evr= IMP.core.NonbondedRestraint(ps, nbl)
evri= m.add_restraint(evr)

# Restraint for bonds
br= IMP.core.BondDecoratorRestraint(IMP.core.Harmonic(0,1), bl)
bri= m.add_restraint(br)

# Just for fun to make the chain straight (angles in radians)
ats= IMP.core.AngleTripletScore(IMP.core.Harmonic(3.1415, .1))
ar= IMP.core.TripletChainRestraint(ats)
ar.add_chain(chain)
ari= m.add_restraint(ar)

# Tie the ends of the chain
# We cound have used a bond instead
p= IMP.ParticlePair(chain[0], chain[-1])
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
vrml= IMP.core.VRMLLogOptimizerState("state.%03d.vrml", chain)
vrml.set_radius_key(rk)
vrml.update()
vrml.set_skip_steps(100)
o.add_optimizer_state(vrml)

# We probably don't need this many steps
o.optimize(1000)

# Write the final state out
vrml.write_next_file()
