## \example restrainer/nup84_complex_in_bead_representation.py
## This example shows basic modeling of Nup84 complex using bead representation. On the following figures, we see XML input files for the representation, restraint, optimization and display. See \ref modules/kernel/nup84_cg.py "Nup84 coarse grained" example in the kernel for a non-restrainer version of this example.
##
## \include nup84_representation.xml
##
## The XML representation input file determines how the system is represented using a 'bead model'. Each protein is represented as a sphere, or a pair of spheres (in the case of the rodlike Nup133 and Nup120 proteins), with larger proteins using larger spheres.
##
## \include nup84_restraint.xml
##
## The XML restraint input file encodes the input structural data as spatial restraints on the system. Here, we use two simple sources of information. First, excluded volume for each protein. Second, yeast two-hybrid results for some pairs of proteins.
##
## \include nup84_optimization.xml
##
## The XML optimization input file sets up a simple conjugate gradients optimization.
##
## \include nup84_display.xml
##
## The XML display input file is for visualization only, and assigns each sphere a different color.
##
##
## The following Python script loads in all four of the XML files and performs the optimization. Restrainer first generates a set of sphere-like particles to represent the system. It then converts the information in the restraints file into a set of IMP restraints. It generates an excluded volume restraint that prevents each protein sphere from penetrating any other sphere and a set of 'connectivity' restraints that force the protein particles to reproduce the interactions implied by the yeast two-hybrid experiments. The optimization generates a file optimized.py that is an input file for the molecular visualization program Chimera; when loaded into Chimera, it displays the final optimized configuration of the complex.
##

#-- File: nup84_complex_in_bead_representation.py --#

import IMP
import IMP.restrainer

IMP.base.set_log_level(IMP.base.VERBOSE)

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
