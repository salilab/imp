import IMP
import IMP.core

m= IMP.Model()
p0= IMP.Particle()
m.add_particle(p0)
d0= IMP.core.XYZRDecorator.create(p0)
d0.set_coordinates(IMP.Vector3D(0,1,2))
d0.set_radius(1.0)
p1= IMP.Particle()
m.add_particle(p1)
d1= IMP.core.XYZRDecorator.create(p1)
d1.set_coordinates(IMP.Vector3D(3,4,5))
d1.set_radius(2.0)

print IMP.core.distance(d0, d1)

# use them as XYZDecorator particles
xd0= IMP.core.XYZDecorator.cast(p0)
xd1= IMP.core.XYZDecorator.cast(p1)

# distance without radii
print IMP.core.distance(xd0, xd1)
