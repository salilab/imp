# Python files placed in this src directory are automatically added to
# the IMP.example module, each as a submodule.
#
# i.e. the function below can be accessed as
# IMP.example.randomize.randomize_particle().


import IMP.core
import IMP.algebra

def randomize_particle(p):
    d = IMP.core.XYZ(p)
    d.set_coordinates(IMP.algebra.get_random_vector_in(
                                   IMP.algebra.get_unit_bounding_box_3d()))
