## \example core/excluded_volume.py
## Setup an excluded volume restraint between a bunch of particles with radius.
##

import IMP.example

(m,c)=IMP.example.create_model_and_particles()

# this container lists all pairs that are close at the time of evaluation
nbl= IMP.container.ClosePairContainer(c, 0,2)
h= IMP.core.HarmonicLowerBound(0,1)
sd= IMP.core.SphereDistancePairScore(h)
# use the lower bound on the inter-sphere distance to push the spheres apart
nbr= IMP.container.PairsRestraint(sd, nbl)
m.add_restraint(nbr)

# alternatively, one could just do
r = IMP.core.ExcludedVolumeRestraint(c)
m.add_restraint(r)

# get the current score
print m.evaluate(False)
