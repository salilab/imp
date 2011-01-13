import IMP.atom

# Use the CHARMM all-atom (i.e. including hydrogens) topology
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
# coordinate information (an extended chain conformation will be produced)
topology.add_atom_types(h)
topology.add_coordinates(h)

# Write out the final structure to a PDB file
IMP.atom.write_pdb(h, 'structure.pdb')
