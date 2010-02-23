import IMP.core

def randomize_particle(p):
    d= IMP.core.XYZ.decorate_particle(p)
    d.set_coordinates(IMP.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
