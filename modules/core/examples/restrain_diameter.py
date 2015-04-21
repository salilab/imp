## \example core/restrain_diameter.py
# An example restraining the diameter of a set of points. That is, the
# restraint penalizes conformations where there are two points more than a
# certain distance from one another.

import IMP
import IMP.core
import IMP.container

# This example restraint the diameter of a set of particles to be smaller
# than 10

diameter = 10
m = IMP.Model()
lc = IMP.container.ListSingletonContainer(
    IMP.core.create_xyzr_particles(m, 50, 1.0))
h = IMP.core.HarmonicUpperBound(0, 1)
r = IMP.core.DiameterRestraint(h, lc, diameter)
sf = IMP.core.RestraintsScoringFunction([r])

# Set up optimizer
o = IMP.core.ConjugateGradients(m)
o.set_scoring_function(sf)

max = 0
for p0 in lc.get_particles():
    for p1 in lc.get_particles():
        d = IMP.core.get_distance(IMP.core.XYZ(p0),
                                  IMP.core.XYZ(p1))
        if d > max:
            max = d
print("The maximum distance is " + str(max))

IMP.set_log_level(IMP.SILENT)
o.optimize(100)

max = 0
for p0 in lc.get_particles():
    for p1 in lc.get_particles():
        d = IMP.core.get_distance(IMP.core.XYZ(p0),
                                  IMP.core.XYZ(p1))
        if d > max:
            max = d
print("Afterwards, the maximum distance is " + str(max))
