## \example kernel/setup.py
## Standard setup code.

import IMP.core

def create_model_and_particles():
    m= IMP.Model()
    ps= [IMP.Particle(m) for i in range(0,100)]
    bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                 IMP.algebra.Vector3D(10,10,10))
    for p in ps:
        d=IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 1))
        d.set_coordinates_are_optimized(True)
    sc= IMP.container.ListSingletonContainer(ps)
    return (m, sc)
