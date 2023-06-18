## \example core/excluded_volume.py
# Setup an excluded volume restraint between a bunch of particles with radius.
#

import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "excluded volume")

# Make 100 particles randomly distributed in a cubic box
m = IMP.Model()
lsc = IMP.container.ListSingletonContainer(m)
b = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))
for i in range(100):
    p = m.add_particle("p")
    lsc.add(p)
    d = IMP.core.XYZR.setup_particle(m, p,
              IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(b), 1))

# this container lists all pairs that are close at the time of evaluation
nbl = IMP.container.ClosePairContainer(lsc, 0, 2)
h = IMP.core.HarmonicLowerBound(0, 1)
sd = IMP.core.SphereDistancePairScore(h)
# use the lower bound on the inter-sphere distance to push the spheres apart
nbr = IMP.container.PairsRestraint(sd, nbl)

# alternatively, one could just do
r = IMP.core.ExcludedVolumeRestraint(lsc)

# get the current score
sf = IMP.core.RestraintsScoringFunction([nbr, r])
print(sf.evaluate(False))
