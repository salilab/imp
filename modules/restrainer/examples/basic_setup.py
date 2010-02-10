#-- File: basic_setup.py --#

import IMP
import IMP.restrainer

# Load molecular hierarchy definition
RepParser = IMP.restrainer.XMLRepresentation(
                IMP.restrainer.get_example_path('input/eg1_representation.xml'))
representation = RepParser.run()

# Load the restraint set
RestraintParser = IMP.restrainer.XMLRestraint(
                     IMP.restrainer.get_example_path('input/eg1_restraint.xml'))
restraint = RestraintParser.run()

# Place representation into IMP model
Model = representation.to_model()

# Place restraint into IMP model
restraint.add_to_representation(representation)

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

Model.show()
Model.evaluate(False)
