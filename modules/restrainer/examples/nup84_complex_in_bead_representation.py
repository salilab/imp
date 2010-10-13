#-- File: nup84_complex_in_bead_representation.py --#

import IMP
import IMP.restrainer

IMP.set_log_level(IMP.VERBOSE)

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation, restraint, optimization and display to restrainer
rep = restrainer.add_representation(IMP.restrainer.get_example_path('input/nup84_representation.xml'))
rsr = restrainer.add_restraint(IMP.restrainer.get_example_path('input/nup84_restraint.xml'))
opt = restrainer.add_optimization(IMP.restrainer.get_example_path('input/nup84_optimization.xml'))
disp = restrainer.add_display(IMP.restrainer.get_example_path('input/nup84_display.xml'))

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

# Save the initial state in Chimera format
restrainer.log.write('initial.py')

# Perform optimization
restrainer.optimize()

# Save the optimized state in Chimera format
restrainer.log.write('optimized.py')
