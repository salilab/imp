#-- File: rigid_body_and_excluded_volume_restraint.py --#

import IMP
import IMP.restrainer

# Load molecular hierarchy definition
RepParser = IMP.restrainer.XMLRepresentation('input/eg2_representation.xml')
representation = RepParser.run()

# Load the restraint set
RestraintParser = IMP.restrainer.XMLRestraint('input/eg2_restraint.xml')
restraint = RestraintParser.run()

# Place representation into IMP model
Model = representation.to_model()

# Place restraint into IMP model
restraint.add_to_representation(representation)


###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###


# Find the IMP data structure with the given id
protein1_hierarchy = representation.find_by_id('Protein1').model_decorator

# Get the rigid body with the given id
protein1_rb = restraint.get_rigid_body_by_id ("Protein1")
protein1_rb.show()

# Define transformation
ub = IMP.algebra.Vector3D(-50.0,-50.0,-50.0)
lb = IMP.algebra.Vector3D( 50.0, 50.0, 50.0)
bb = IMP.algebra.BoundingBox3D(ub, lb)
translation = IMP.algebra.random_vector_in_box(bb)
rotation = IMP.algebra.random_rotation()
transformation = IMP.algebra.Transformation3D(rotation, translation)

# Perform geometric transformations on the Protein1 rigid body
protein1_rb.set_transformation(transformation)
