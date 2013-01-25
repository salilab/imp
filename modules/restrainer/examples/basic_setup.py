## \example restrainer/basic_setup.py
## This example shows how to construct a molecular hierarchy consisting of two proteins, and then apply connectivity restraint obtained from pulldown experimental data to these two proteins.
##
## On the following figure, we see an XML representation of the molecular hierarchy.
##
## \include eg1_representation.xml
##
## Each level of molecular hierarchy corresponds to an XML tag. Each tag can have
## optional attributes. In this example, each protein has a unique id. Protein1 has
## 2 chains. The first chain is constructed from the PDB file using
## IMP::atom::CAlphaSelector. The second chain is represented by a sphere fragment
## that is big enough to contain 10 residues. Protein2 is constructed using the
## default selector, IMP::atom::NonWaterNonHydrogenSelector.
##
## The following figure shows the definition of the restraint.
##
## \include eg1_restraint.xml
##
## The pulldown restraint is applied to Protein1 and Protein2.
##
## The following Python script demonstrates the process of loading our data into \imp model.
##

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
model = restrainer.get_model()

model.show()
model.evaluate(False)
