import IMP
import IMP.core
import IMP.atom
import IMP.helper

# This example addes a restraint on nonbonded interactions
# Since it is between two rigid bodies, internal interactions are ignored

m= IMP.Model()
# The particles in the rigid bodies
rbps0= IMP.core.create_xyzr_particles(m, 3, 1)
rbps1= IMP.core.create_xyzr_particles(m, 3, 1)

rbp0= IMP.Particle(m)
rbp1= IMP.Particle(m)

rbss0 = IMP.core.RigidBody.setup_particle(rbp0, IMP.core.XYZs(rbps0))

rbss1 = IMP.core.RigidBody.setup_particle(rbp1, IMP.core.XYZs(rbps1))

lsc= IMP.core.ListSingletonContainer()
lsc.add_particle(rbp0)
lsc.add_particle(rbp1)

# Set up the nonbonded list
nbl= IMP.core.ClosePairContainer(lsc, 0, IMP.core.RigidClosePairsFinder(IMP.core.RigidMembersRefiner()), 2.0)

# Set up excluded volume
ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1))
evr= IMP.core.PairsRestraint(ps, nbl)
evri= m.add_restraint(evr)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

o.optimize(1000)
