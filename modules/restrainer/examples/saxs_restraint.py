## \example restrainer/saxs_restraint.py
## This example shows how to use IMP::saxs::Restraint.
##
## \include saxs_representation.xml
## \include saxs_restraint.xml
##

#-- File: saxs_restraint.py --#

import IMP
import IMP.restrainer

# Create restrainer object
restrainer = IMP.restrainer.Main()

# Add representation and restraint to restrainer
rep = restrainer.add_representation(IMP.restrainer.get_example_path('input/saxs_representation.xml'))
rsr = restrainer.add_restraint(IMP.restrainer.get_example_path('input/saxs_restraint.xml'))

###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###

# Get the IMP model object used by restrainer
model = restrainer.get_model()

# Get restraint by name
r = rsr.get_restraint_by_name('saxs_restraint')

# Get IMP::RestraintSet by name
rs = rsr.get_restraint_set_by_name('saxs_restraint_set')

# Get IMP::saxs::Restraint
saxs_restraint = r.imp_restraint
saxs_restraint.evaluate(False)
