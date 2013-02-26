## \example atom/charmm_forcefield.py
## In this example, a PDB file is read in and scored using the CHARMM forcefield. For more control over the setup of the forcefield, see the 'charmm_forcefield_verbose.py' example.
##

import IMP.atom
import IMP.container

# Create an IMP model and add a heavy atom-only protein from a PDB file
m = IMP.Model()
prot = IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m,
                         IMP.atom.NonWaterNonHydrogenPDBSelector())

# Read in the CHARMM heavy atom topology and parameter files
ff = IMP.atom.get_heavy_atom_CHARMM_parameters()

# Using the CHARMM libraries, determine the ideal topology (atoms and their
# connectivity) for the PDB file's primary sequence
topology = ff.create_topology(prot)

# Typically this modifies the C and N termini of each chain in the protein by
# applying the CHARMM CTER and NTER patches. Patches can also be manually
# applied at this point, e.g. to add disulfide bridges.
topology.apply_default_patches()

# Make the PDB file conform with the topology; i.e. if it contains extra
# atoms that are not in the CHARMM topology file, remove them; if it is
# missing atoms (e.g. sidechains, hydrogens) that are in the CHARMM topology,
# add them and construct their Cartesian coordinates from internal coordinate
# information.
topology.setup_hierarchy(prot)

# Set up and evaluate the stereochemical part (bonds, angles, dihedrals,
# impropers) of the CHARMM forcefield
r = IMP.atom.CHARMMStereochemistryRestraint(prot, topology)
m.add_restraint(r)

# Add non-bonded interaction (in this case, Lennard-Jones). This needs to
# know the radii and well depths for each atom, so add them from the forcefield
# (they can also be assigned manually using the XYZR or LennardJones
# decorators):
ff.add_radii(prot)
ff.add_well_depths(prot)

# Get a list of all atoms in the protein, and put it in a container
atoms = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
cont = IMP.container.ListSingletonContainer(atoms)

# Add a restraint for the Lennard-Jones interaction. This is built from
# a collection of building blocks. First, a ClosePairContainer maintains a list
# of all pairs of Particles that are close. Next, all 1-2, 1-3 and 1-4 pairs
# from the stereochemistry created above are filtered out.
# Then, a LennardJonesPairScore scores a pair of atoms with the Lennard-Jones
# potential. Finally, a PairsRestraint is used which simply applies the
# LennardJonesPairScore to each pair in the ClosePairContainer.
nbl = IMP.container.ClosePairContainer(cont, 4.0)
nbl.add_pair_filter(r.get_pair_filter())

sf = IMP.atom.ForceSwitch(6.0, 7.0)
ps = IMP.atom.LennardJonesPairScore(sf)
m.add_restraint(IMP.container.PairsRestraint(ps, nbl))

# it gets awfully slow with internal checks
IMP.base.set_check_level(IMP.base.USAGE)
# Finally, evaluate the score of the whole system (without derivatives)
print m.evaluate(False)
