## \example core/randomize_rigid_body.py
## This fragments shows how to either perturb or set the orientation of a rigid
## body randomly.

import IMP.core
import IMP.algebra
import IMP
from IMP.algebra import ReferenceFrame3D
from IMP.algebra import Transformation3D

m = IMP.Model()
p = IMP.Particle(m)
rbd = IMP.core.RigidBody.setup_particle(p, ReferenceFrame3D())
translation=IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())

# we don't yet have python code to generate a nearby rotation
rotation= IMP.algebra.get_random_rotation_3d()
transformation= Transformation3D(rotation, translation)
# Option 1:
# note, this overwrites the existing position
# The True is to transform the members now rather than wait for a
# score state
rbd.set_reference_frame(ReferenceFrame3D(transformation))
# Option 2:
# perturb the existing transformation
composed_tr = IMP.algebra.compose \
                       (rbd.get_reference_frame().get_transformation_to(),
                        transformation)
rbd.set_reference_frame(ReferenceFrame3D(composed_tr))
# Alternative to Option 2:
IMP.core.transform(rbd, transformation)
