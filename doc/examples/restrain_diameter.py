import IMP
import IMP.core

# This example restraints the diameter of a set of particles to be smaller than 10

diameter=10
m= IMP.Model()
lc= IMP.core.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 20, 1.0))
# create a list of all pairs of particles from lc
ps = IMP.core.AllPairsPairContainer(lc)

h=IMP.core.HarmonicUpperBound(diameter,1)
d=IMP.core.DistancePairScore(h)
# apply d to each pair from ps
r= IMP.core.PairsRestraint(d, ps)
m.add_restraint(r)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

o.optimize(100)

for i in range(0, ps.get_number_of_particle_pairs()):
    p= ps.get_particle_pair(i)
    print IMP.core.distance(IMP.core.XYZDecorator(p[0]),
                            IMP.core.XYZDecorator(p[1]))
