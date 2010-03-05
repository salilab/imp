#-- File: basic_display.py --#

import IMP
import IMP.restrainer

# Load molecular hierarchy definition
rep_parser = IMP.restrainer.XMLRepresentation(
                IMP.restrainer.get_example_path('input/display_representation.xml'))
representation = rep_parser.run()

# Load color definition
display_parser = IMP.restrainer.XMLDisplay(
                    IMP.restrainer.get_example_path('input/pdb_display.xml'))
display = display_parser.run()

# Place representation into IMP model
model = representation.to_model()

# Write initial display in Chimera format
log = display.create_log(representation, 'pdb_log_%03d.py')
log.write('initial_display_in_chimera.py')

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

model.show()
model.evaluate(False)
