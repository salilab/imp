import IMP
import IMP.core
import IMP.algebra

m= IMP.Model()
p0= IMP.Particle(m)
d0= IMP.core.XYZDecorator.create(p0, IMP.algebra.Vector3D(0,1,2))
p1= IMP.Particle(m)
d1= IMP.core.XYZDecorator.create(p1)
d1.set_coordinates(IMP.algebra.Vector3D(3,4,5))

print IMP.core.distance(d0, d1)
