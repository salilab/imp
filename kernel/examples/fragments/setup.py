import IMP.core

def create_model_and_particles():
    m= IMP.Model()
    sc= IMP.container.ListSingletonContainer()
    b= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                 IMP.algebra.Vector3D(10,10,10))
    for i in range(0,100):
        p= IMP.Particle(m)
        sc.add_particle(m)
        d=IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(b), 1))
        d.set_coordinates_are_optimized(True)
    return (m, sc)
