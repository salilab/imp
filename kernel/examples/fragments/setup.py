import IMP
import IMP.core
m= IMP.Model()
ps= IMP.core.SingletonContainer()
for i in range(0,100)
    p= IMP.Particle(m)
    d= IMP.core.XYZ.setup_particle(p, IMP.get_random_vector_in(), 1.0)
    d.set_coordinates_are_optimized(True)
    ps.add_particle(p)
