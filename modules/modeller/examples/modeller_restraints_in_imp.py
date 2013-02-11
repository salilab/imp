## \example modeller/modeller_restraints_in_imp.py
## This demonstrates using Modeller restraints as additional terms in the IMP
## scoring function, so that existing Modeller restraints can be used in
## combination with new IMP restraints and optimization protocols.
##

import modeller
import IMP
import IMP.modeller

# Set up Modeller and build a model from the GGCC primary sequence
e = modeller.environ()
e.edat.dynamic_sphere = False
e.libs.topology.read('${LIB}/top_heav.lib')
e.libs.parameters.read('${LIB}/par.lib')
modmodel = modeller.model(e)
modmodel.build_sequence('GGCC')

# Add a simple Modeller distance restraint between the first and last atoms
feat = modeller.features.distance(modmodel.atoms[0], modmodel.atoms[-1])
r = modeller.forms.gaussian(feature=feat, mean=10.0, stdev=1.0,
                            group=modeller.physical.xy_distance)
modmodel.restraints.add(r)

# Set up IMP and load the Modeller model in as a new Hierarchy
m = IMP.Model()
protein = IMP.modeller.ModelLoader(modmodel).load_atoms(m)
atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)

# Use the ModellerRestraints class to add all of the Modeller restraints to
# the IMP scoring function
m.add_restraint(IMP.modeller.ModellerRestraints(m, modmodel,
                                                atoms))

# Calculate the IMP score
print m.evaluate(False)
