## \example core/restrain_diameter.py
## An example restraining the diameter of a set of points. That is, the restraint penalizes conformations where there are two point more than a certain distance from one another.

import IMP
import IMP.core
import IMP.container

# This example restraints the diameter of a set of particles to be smaller than 10

diameter=10
m= IMP.Model()
lc= IMP.container.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 50, 1.0))
h=IMP.core.HarmonicUpperBound(0,1)
r=IMP.core.DiameterRestraint(h, lc, diameter)
m.add_restraint(r)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

max=0
for p0 in lc.get_particles():
    for p1 in lc.get_particles():
        d=IMP.core.get_distance(IMP.core.XYZ(p0),
                            IMP.core.XYZ(p1))
        if d > max: max=d
print "The maximim distance is "+str(max)

IMP.base.set_log_level(IMP.base.SILENT)
o.optimize(100)

max=0
for p0 in lc.get_particles():
    for p1 in lc.get_particles():
        d=IMP.core.get_distance(IMP.core.XYZ(p0),
                            IMP.core.XYZ(p1))
        if d > max: max=d
print "Afterwards, the maximim distance is "+str(max)
