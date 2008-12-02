import IMP
import IMP.core

# This example addes a restraint on nonbonded interactions
# after excluding a set of bonded interactions.

m= IMP.Model()
# The set of particles
ps = IMP.core.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 20, 1.0))

# create a bond between two particles
bonds= IMP.core.ListSingletonContainer()
bd0= IMP.core.BondedDecorator.create(ps.get_particle(0))
bd1= IMP.core.BondedDecorator.create(ps.get_particle(1))
bonds.add_particle(IMP.core.custom_bond(bd0, bd1, 2.0).get_particle())

# Set up the nonbonded list
nbl= IMP.core.ClosePairsScoreState(ps)
m.add_score_state(nbl)
# Exclude bonds from closest pairs
fl= nbl.get_close_pairs_container()
fl.add_pair_container(IMP.core.BondDecoratorPairContainer())

# Set up excluded volume
ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1))
evr= IMP.core.PairsRestraint(ps, fl)
evri= m.add_restraint(evr)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

o.optimize(1000)
