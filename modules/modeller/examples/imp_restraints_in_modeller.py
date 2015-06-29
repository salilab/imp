## \example modeller/imp_restraints_in_modeller.py
# This demonstrates using IMP.Restraints as additional energy terms in the
# Modeller scoring function, so that IMP scoring terms can be incorporated into
# existing comparative modeling pipelines.
#

import modeller
import IMP
import IMP.core
import IMP.modeller
import sys

IMP.setup_from_argv(sys.argv, "IMP restraints in Modeller")

# Set up Modeller and build a model from the GGCC primary sequence
e = modeller.environ()
e.edat.dynamic_sphere = False
e.libs.topology.read('${LIB}/top_heav.lib')
e.libs.parameters.read('${LIB}/par.lib')
modmodel = modeller.model(e)
modmodel.build_sequence('GGCC')

# Set up IMP and load the Modeller model in as a new Hierarchy
m = IMP.Model()
protein = IMP.modeller.ModelLoader(modmodel).load_atoms(m)

# Create a simple IMP distance restraint between the first and last atoms
atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
r = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(10.0, 1.0),
                               atoms[0].get_particle(),
                               atoms[-1].get_particle())
sf = IMP.core.RestraintsScoringFunction([r])

# Use the IMPRestraints class to add this IMP scoring function to the
# Modeller scoring function
t = modmodel.env.edat.energy_terms
t.append(IMP.modeller.IMPRestraints(atoms, sf))

# Calculate the Modeller energy (score) for the whole protein
sel = modeller.selection(modmodel)
sel.energy()
