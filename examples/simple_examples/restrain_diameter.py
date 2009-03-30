import IMP
import IMP.core

# This example restraints the diameter of a set of particles to be smaller than 10

diameter=10
m= IMP.Model()
lc= IMP.core.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 50, 1.0))
h=IMP.core.HarmonicUpperBound(0,1)
r=IMP.core.DiameterRestraint(h, lc, diameter)
m.add_restraint(r)

# Set up optimizer
o= IMP.core.ConjugateGradients()
o.set_model(m)

max=0
for p0 in lc.get_particles():
    for p1 in lc.get_particles():
        d=IMP.core.distance(IMP.core.XYZDecorator(p0),
                            IMP.core.XYZDecorator(p1))
        if d > max: max=d
print "The maximim distance is "+str(max)

IMP.set_log_level(IMP.SILENT)
o.optimize(100)

max=0
for p0 in lc.get_particles():
    for p1 in lc.get_particles():
        d=IMP.core.distance(IMP.core.XYZDecorator(p0),
                            IMP.core.XYZDecorator(p1))
        if d > max: max=d
print "Afterwards, the maximim distance is "+str(max)
