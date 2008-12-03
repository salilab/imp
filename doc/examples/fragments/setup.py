import IMP
import IMP.core
m= IMP.Model()
ps= IMP.core.SingletonContainer()
for i in range(0,100)
    p= IMP.Particle()
    m.add_particle(p)
    d= IMP.core.XYZDecorator.create(p)
    d.set_coordinates_are_optimized(True)
    d.set_coordinates(IMP.random_vector_in_box())
    d.set_radius(1.0)
    ps.add_particle(p)
