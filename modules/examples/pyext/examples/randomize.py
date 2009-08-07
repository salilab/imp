import IMP.core

def randomize_particle(p):
    d= IMP.core.XYZ.decorate_particle(p)
    d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
