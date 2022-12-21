## \example core/pair_restraint.py
# Restrain the distance between a pair of particles.
#

import IMP.algebra
import IMP.core
import sys

IMP.setup_from_argv(sys.argv, "pair restraint")

# Make two xyz particles randomly distributed in space
m = IMP.Model()
b = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))
inds = []
for i in range(2):
    p = m.add_particle("p")
    d = IMP.core.XYZR.setup_particle(m, p,
                  IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(b), 1))
    inds.append(p)

# Score the distance between the two particles with a harmonic function
uf = IMP.core.Harmonic(0, 1)
df = IMP.core.DistancePairScore(uf)
r = IMP.core.PairRestraint(m, df, (inds[0], inds[1]))

# Print the current score of the restraint, without first derivatives
print(r.evaluate(False))
