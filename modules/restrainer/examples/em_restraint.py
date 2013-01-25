## \example restrainer/em_restraint.py
## This example shows how to use IMP::em::FitRestraint.
##
## \include em_representation.xml
## \include em_restraint.xml
##

#-- File: em_restraint.py --#

import IMP
import IMP.restrainer

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation and restraint to restrainer
rep = restrainer.add_representation(IMP.restrainer.get_example_path('input/em_representation.xml'))
rsr = restrainer.add_restraint(IMP.restrainer.get_example_path('input/em_restraint.xml'))

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

# Get the IMP model object used by restrainer
model = restrainer.get_model()

# Get restraint by name
r = rsr.get_restraint_by_name('em_restraint')

# Get EM density map header from the restraint
dmap_header = r.dmap_header

# Get IMP::em::FitRestraint
fit_restraint = r.imp_restraint
fit_restraint.evaluate(False)
