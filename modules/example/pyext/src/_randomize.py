# python files placed in this src directory are automatically added to the module.
# The function below can be accessed as
# IMP.example.randomize.randomize_particles().

import IMP.core


def randomize_particle(p):
    d = IMP.core.XYZ(p)
    d.set_coordinates(IMP.algebra.get_random_vector_in(
        IMP.algebra.get_unit_bounding_box_3d()))
