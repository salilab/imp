## \example core/restrain_diameter.py
# An example restraining the diameter of a set of points. That is, the
# restraint penalizes conformations where there are two points more than a
# certain distance from one another.

import IMP
import IMP.core
import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "restrain diameter")

# This example restraint the diameter of a set of particles to be smaller
# than 10

diameter = 10
m = IMP.Model()
lc = IMP.container.ListSingletonContainer(
    m, IMP.core.create_xyzr_particles(m, 50, 1.0))
h = IMP.core.HarmonicUpperBound(0, 1)
r = IMP.core.DiameterRestraint(h, lc, diameter)
sf = IMP.core.RestraintsScoringFunction([r])

# Set up optimizer
o = IMP.core.ConjugateGradients(m)
o.set_scoring_function(sf)

max = 0
for pi0 in lc.get_contents():
    for pi1 in lc.get_contents():
        d = IMP.core.get_distance(IMP.core.XYZ(m, pi0),
                                  IMP.core.XYZ(m, pi1))
        if d > max:
            max = d
print("The maximum distance is " + str(max))

IMP.set_log_level(IMP.SILENT)
o.optimize(100)

max = 0
for pi0 in lc.get_contents():
    for pi1 in lc.get_contents():
        d = IMP.core.get_distance(IMP.core.XYZ(m, pi0),
                                  IMP.core.XYZ(m, pi1))
        if d > max:
            max = d
print("Afterwards, the maximum distance is " + str(max))
