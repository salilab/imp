## \example core/randomize_rigid_body.py
# This fragment shows how to either perturb or set the orientation of a rigid
# body randomly.

import IMP.core
import IMP.algebra
import IMP

m = IMP.kernel.Model()
p = IMP.kernel.Particle(m)
rbd = IMP.core.RigidBody.setup_particle(p, IMP.algebra.ReferenceFrame3D())
translation = IMP.algebra.get_random_vector_in(
    IMP.algebra.get_unit_bounding_box_3d())

# we don't yet have python code to generate a nearby rotation
rotation = IMP.algebra.get_random_rotation_3d()
transformation = IMP.algebra.Transformation3D(rotation, translation)
# Option 1:
# note, this overwrites the existing position
rbd.set_reference_frame(IMP.algebra.ReferenceFrame3D(transformation))
# Option 2:
# perturb the existing transformation
composed_tr = IMP.algebra.compose \
    (rbd.get_reference_frame().get_transformation_to(),
     transformation)
rbd.set_reference_frame(IMP.algebra.ReferenceFrame3D(composed_tr))
# Alternative to Option 2:
IMP.core.transform(rbd, transformation)
