# python files placed in this src directory are automatically added to the module.
# The function below can be accessed as IMP.examples.randomize.randomize_particles().

import IMP.core

def randomize_particle(p):
    d= IMP.core.XYZ.decorate_particle(p)
    d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
