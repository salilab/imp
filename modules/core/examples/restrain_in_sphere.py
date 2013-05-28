## \example core/restrain_in_sphere.py
## This fragment shows how to restrain a set of points stored in a SingletonContainer in a sphere of radius 'radius' centered around 'center'.

import IMP
import IMP.core
radius=10
stiffness=2
center= IMP.algebra.Vector3D(1,2,3)
m = IMP.Model()
p = m.add_particle("particle")
IMP.core.XYZR.setup_particle(m, p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(100,0,0), 1))

ub= IMP.core.HarmonicUpperBound(radius, stiffness)

# Restrain based on the distance to a single point (hence a ball
ss= IMP.core.DistanceToSingletonScore(ub, center)

r= IMP.core.SingletonRestraint(ss, [m.get_particle(p)])
r.evaluate(False)
