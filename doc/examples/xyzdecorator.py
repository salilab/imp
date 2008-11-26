import IMP
import IMP.core

m= IMP.Model()
p0= IMP.Particle()
m.add_particle(p0)
d0= IMP.core.XYZDecorator.create(p0)
d0.set_coordinates(IMP.Vector3D(0,1,2))
p1= IMP.Particle()
m.add_particle(p1)
d1= IMP.core.XYZDecorator.create(p1)
d1.set_coordinates(IMP.Vector3D(3,4,5))

print IMP.core.distance(d0, d1)
