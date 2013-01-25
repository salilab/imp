## \example core/XYZ_Decorator.py
## This is a simple example using the XYZDecorator to set the coordinates of some particles and compute the distance between them.

import IMP
import IMP.core
import IMP.algebra

m= IMP.Model()
p0= IMP.Particle(m)
# add x,y,z coordinates to the particle
d0= IMP.core.XYZ.setup_particle(p0, IMP.algebra.Vector3D(0,1,2))
p1= IMP.Particle(m)
# add slots for the coordinates, but leave them uninitialized
d1= IMP.core.XYZ.setup_particle(p1)
# set the coordinate values
d1.set_coordinates(IMP.algebra.Vector3D(3,4,5))

# return the distance between the two points described by the decorated
# particles
print IMP.core.get_distance(d0, d1)
