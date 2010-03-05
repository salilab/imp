#-- File: basic_setup.py --#

import IMP
import IMP.restrainer

# Load molecular hierarchy definition
rep_parser = IMP.restrainer.XMLRepresentation(
                IMP.restrainer.get_example_path('input/eg1_representation.xml'))
representation = rep_parser.run()

# Load the restraint set
restraint_parser = IMP.restrainer.XMLRestraint(
                     IMP.restrainer.get_example_path('input/eg1_restraint.xml'))
restraint = restraint_parser.run()

# Place representation into IMP model
model = representation.to_model()

# Place restraint into IMP model
restraint.add_to_representation(representation)

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

model.show()
model.evaluate(False)
