## \example atom/structure_from_sequence.py
## An atomic protein structure is created from primary (amino-acid) sequence.
##

import IMP.atom

# Use the CHARMM all-atom (i.e. including hydrogens) topology and parameters
topology = IMP.atom.CHARMMTopology(IMP.atom.get_all_atom_CHARMM_parameters())

# Create a single chain of amino acids and apply the standard C- and N-
# termini patches
topology.add_sequence('IACGACKPECPVNIIQGS')
topology.apply_default_patches()

# Make an IMP Hierarchy (atoms, residues, chains) that corresponds to
# this topology
m = IMP.Model()
h = topology.create_hierarchy(m)

# Generate coordinates for all atoms in the Hierarchy, using CHARMM internal
# coordinate information (an extended chain conformation will be produced).
# Since in some cases this information can be incomplete, better results will
# be obtained if the atom types are assigned first and the CHARMM parameters
# file is loaded, as we do here, so missing information can be filled in.
# It will still work without that information, but will approximate the
# coordinates.
topology.add_atom_types(h)
topology.add_coordinates(h)

# Hierarchies in IMP must have radii
IMP.atom.add_radii(h)

# Write out the final structure to a PDB file
IMP.atom.write_pdb(h, 'structure.pdb')
