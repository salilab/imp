## \example core/XYZR_Decorator.py
## This is a simple example using the XYZRDecorator to set the coordinates and radius of a particle and compute distances between the resulting spheres.

import IMP
import IMP.core
import IMP.algebra

m= IMP.Model()
p0= IMP.Particle(m)
d0= IMP.core.XYZR.setup_particle(p0, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,1,2),
                                                           1.0))
p1= IMP.Particle(m)
d1= IMP.core.XYZR.setup_particle(p1)
d1.set_coordinates(IMP.algebra.Vector3D(3,4,5))
d1.set_radius(2.0)

print IMP.core.get_distance(d0, d1)

# use them as XYZ particles
xd0= IMP.core.XYZ.decorate_particle(p0)
xd1= IMP.core.XYZ.decorate_particle(p1)

# distance without radii
print IMP.core.get_distance(xd0, xd1)
