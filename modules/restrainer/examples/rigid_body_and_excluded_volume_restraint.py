#-- File: rigid_body_and_excluded_volume_restraint.py --#

import IMP
import IMP.restrainer

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation and restraint to restrainer
rep = restrainer.add_representation(IMP.restrainer.get_example_path('input/eg2_representation.xml'))
rsr = restrainer.add_restraint(IMP.restrainer.get_example_path('input/eg2_restraint.xml'))

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

# Get the IMP model object used by restrainer
model = restrainer.model()

# Find the IMP data structure with the given id
protein1_hierarchy = rep.get_imp_hierarchy_by_id('Protein1')

# Get root hierarchy
root_hierarchy = rep.get_root_imp_hierarchy()

# Get the rigid body with the given id
protein1_rb = rsr.get_rigid_body_by_id ("Protein1")

# Get all rigid bodies
rbs = rsr.get_all_rigid_bodies()

# Define transformation
ub = IMP.algebra.Vector3D(-50.0,-50.0,-50.0)
lb = IMP.algebra.Vector3D( 50.0, 50.0, 50.0)
bb = IMP.algebra.BoundingBox3D(ub, lb)
translation = IMP.algebra.get_random_vector_in(bb)
rotation = IMP.algebra.get_random_rotation_3d()
transformation = IMP.algebra.Transformation3D(rotation, translation)

# Perform geometric transformations on the Protein1 rigid body
protein1_rb.set_transformation(transformation)
protein1_rb.show()
