#-- File: basic_display.py --#

import IMP
import IMP.restrainer

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation and color definition to restrainer
rep = restrainer.add_representation(IMP.restrainer.get_example_path('input/display_representation.xml'))
disp = restrainer.add_display(IMP.restrainer.get_example_path('input/pdb_display.xml'))

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

# Write initial display in Chimera format
restrainer.log.write('initial_display_in_chimera.py')

# Get the IMP model object used by restrainer
model = restrainer.get_model()

model.show()
model.evaluate(False)
