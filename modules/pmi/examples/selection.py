## \example pmi/selection.py
"""This script demonstrates a few different ways you can perform selection
   in PMI.
   In PMI we first set up molecules at various resolutions.
   Then you call System.build() and this creates all requested representations.
"""

import IMP
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints
import IMP.pmi.restraints.stereochemistry
import sys

IMP.setup_from_argv(sys.argv, "Selection in PMI")

# ############### BEFORE BUILDING ###################
# Preliminaries: read sequences, create a system and a state
seqs = IMP.pmi.topology.Sequences(IMP.pmi.get_example_path('data/1WCM.fasta'))
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st = s.create_state()

# Create a molecule. This sets up "TempResidues" for all elements in
# the sequence.
mol = st.create_molecule("Rpb4", seqs["1WCM:D"], chain_id="D")

# No structure has been built yet - you have to call
# 'Molecule.add_representation'
#  The first argument to that function is a set of "TempResidues"
#  Here are some ways of getting them:

# Slice a molecule (uses python-style 0-ordering)
myres1 = mol[0:10]

# Use PDB numbering with residue_range (inclusive on both ends)
myres2 = mol.residue_range('1', '10')

# If you add a PDB, this returns the set of TempResidues that were in
# the PDB file
atomic = mol.add_structure(IMP.pmi.get_example_path('data/1WCM_fitted.pdb'),
                           chain_id="D")

# You can also call these helper functions at any time
atomic = mol.get_atomic_residues()
non_atomic = mol.get_non_atomic_residues()

# All of the above objects are OrderedSets, so you can actually perform
# set operations
myres3 = mol[0:50] - atomic  # non-atomic residues from 0 to 50
myres4 = mol[0:50] & atomic  # atomic residues between 0 and 50

# Finally, pass your favorite handle to add_representation
mol.add_representation(
    myres3,
    # creates beads with size 1 (non-atomic can only have one beadsize)
    resolutions=[1])
mol.add_representation(myres4,
                       resolutions=[1, 10])  # creates beads with size 1 and 10

# When you have decided all representations, call build()
#  This returns an IMP hierarchy
hier = s.build()

# View your creation with this function
IMP.atom.show_with_representations(hier)


# ############### AFTER BUILDING ###################
# After building, only what you requested with add_representation()
# can be selected

# PMI selection
# Most PMI functions will let you pass Molecules or TempResidues and it
# will automatically gather things.
conn = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol,
                                                                resolution=1)

# Similarly for rigid body creation, we recommend passing PMI objects
# and it will gather all resolutions
dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
dof.create_rigid_body(mol, nonrigid_parts=non_atomic,
                      resolution='all')

# Advanced: IMP selection
# Note if you request a resolution this will find the NEAREST available
# resolution.
#  e.g. if only resolution 1 is built, those particles will be returned below:
sel = IMP.atom.Selection(hier, resolution=10, molecule="Rpb4",
                         residue_indexes=range(1, 10))
particles = sel.get_selected_particles()

# Retrieving the molecule object
# The molecules are stored within the state, so you can do :
all_mol_copies = st.molecules["Rpb4"]  # a list of all copies
mol = all_mol_copies[0]
