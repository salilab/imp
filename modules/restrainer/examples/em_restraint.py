#-- File: em_restraint.py --#

import IMP
import IMP.restrainer

# Load molecular hierarchy definition
RepParser = IMP.restrainer.XMLRepresentation(
                IMP.restrainer.get_example_path('input/em_representation.xml'))
representation = RepParser.run()

# Load the restraint set
RestraintParser = IMP.restrainer.XMLRestraint(
                      IMP.restrainer.get_example_path('input/em_restraint.xml'))
restraint = RestraintParser.run()

# Place representation into IMP model
Model = representation.to_model()

# Place restraint into IMP model
restraint.add_to_representation(representation)


###=======================================================================###
#  At this point all data from XML files have been placed into the model.
#  Now it is possible to perform various operations on the IMP model.
###=======================================================================###


# Get restraint by name
r = restraint.get_restraint_by_name('em_restraint')

# Get EM density map header from the restraint
dmap_header = r.dmap_header

# Get IMP::em::FitRestraint
fit_restraint = r.imp_restraint
fit_restraint.evaluate(False)
