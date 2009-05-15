import IMP.core

def randomize_particle(p):
    d= IMP.core.XYZ.cast(p)
    d.set_coordinates(IMP.random_vector_in_unit_box())
