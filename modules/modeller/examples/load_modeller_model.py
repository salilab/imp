## \example modeller/load_modeller_model.py
# This demonstrates reading in an existing Modeller model, and converting the
# Modeller restraints (both static and dynamic) into equivalent IMP restraints.
#

from __future__ import print_function
import modeller
import IMP
import IMP.modeller
import sys

IMP.setup_from_argv(sys.argv, "Load Modeller model")

# Set up Modeller and build a model from the GGCC primary sequence
e = modeller.Environ()
e.edat.dynamic_sphere = True
e.libs.topology.read('${LIB}/top_heav.lib')
e.libs.parameters.read('${LIB}/par.lib')
modmodel = modeller.Model(e)
modmodel.build_sequence('GGCC')
# Generate Modeller stereochemistry
sel = modeller.Selection(modmodel)
modmodel.restraints.make(sel, restraint_type='STEREO', spline_on_site=False)

# Set up IMP and use the ModelLoader class to load the atom coordinates
# from Modeller into IMP as a new Hierarchy
m = IMP.Model()
loader = IMP.modeller.ModelLoader(modmodel)
protein = loader.load_atoms(m)

# Load each Modeller static restraint, and each Modeller dynamic restraint
# (soft-sphere in this case) in as an IMP.Restraint
sf = IMP.core.RestraintsScoringFunction(
    list(loader.load_static_restraints())
    + list(loader.load_dynamic_restraints()))

print(sf.evaluate(False))
