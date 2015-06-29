## \example core/excluded_volume.py
# Setup an excluded volume restraint between a bunch of particles with radius.
#

import IMP.example
import sys

IMP.setup_from_argv(sys.argv, "excluded volume")

(m, c) = IMP.example.create_model_and_particles()

# this container lists all pairs that are close at the time of evaluation
nbl = IMP.container.ClosePairContainer(c, 0, 2)
h = IMP.core.HarmonicLowerBound(0, 1)
sd = IMP.core.SphereDistancePairScore(h)
# use the lower bound on the inter-sphere distance to push the spheres apart
nbr = IMP.container.PairsRestraint(sd, nbl)

# alternatively, one could just do
r = IMP.core.ExcludedVolumeRestraint(c)

# get the current score
sf = IMP.core.RestraintsScoringFunction([nbr, r])
print(sf.evaluate(False))
