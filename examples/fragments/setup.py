import IMP
import IMP.core
m= IMP.Model()
ps= IMP.core.SingletonContainer()
for i in range(0,100)
    p= IMP.Particle(m)
    d= IMP.core.XYZDecorator.create(p, IMP.random_vector_in_box(), 1.0)
    d.set_coordinates_are_optimized(True)
    ps.add_particle(p)
