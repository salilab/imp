#-- File: basic_setup.py --#

import IMP
import IMP.restrainer

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation and restraint to restrainer
rep = restrainer.add_representation(IMP.restrainer.get_example_path('input/eg1_representation.xml'))
rsr = restrainer.add_restraint(IMP.restrainer.get_example_path('input/eg1_restraint.xml'))

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

# Get the IMP model object used by restrainer
model = restrainer.model()

model.show()
model.evaluate(False)
