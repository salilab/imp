## \example restrainer/basic_display.py
## In case we would like to create Chimera logs storing optimization states, we can provide geometric definitions in a display XML file. These XML definitions form a basic interface to IMP::display module. Currently, the only geometric aspect that can be specified is color.
##
## This example shows how to color the representation components and output display file in chimera format.
##
## \include display_representation.xml
##
## \include pdb_display.xml
##
## Loading a display XML file is similar to loading the representation and restraint XML definitions. Only particles mentioned in the display XML file will be present (this is useful if we only want to display some portion of an hierarchy). The arguments to the create_log are the representation and the file name template (this '%%03d' will be replaced by consecutive numbers if we do optimization). This log is intended to be added to an optimization by performing: optimization.add_optimizer_state(log)
##

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
