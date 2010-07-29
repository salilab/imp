import IMP.atom
import IMP.container

# Create an IMP model and add a heavy atom-only protein from a PDB file
m = IMP.Model()
prot = IMP.atom.read_pdb(IMP.atom.get_example_path("example_protein.pdb"), m,
                         IMP.atom.NonWaterNonHydrogenPDBSelector())

# Read in the CHARMM heavy atom topology and parameter files
ff = IMP.atom.get_default_CHARMM_parameters()

# Using the CHARMM libraries, determine the ideal topology (atoms and their
# connectivity) for the PDB file's primary sequence
topology = ff.create_topology(prot)

# Typically this modifies the C and N termini of each chain in the protein by
# applying the CHARMM CTER and NTER patches. Patches can also be manually
# applied at this point, e.g. to add disulfide bridges.
topology.apply_default_patches(ff)

# Each atom is mapped to its CHARMM type. These are needed to look up bond
# lengths, Lennard-Jones radii etc. in the CHARMM parameter file. Atom types
# can also be manually assigned at this point using the CHARMMAtom decorator.
topology.add_atom_types(prot)

# Generate and return lists of bonds, angles, dihedrals and impropers for
# the protein. Each is a Particle in the model which defines the 2, 3 or 4
# atoms that are bonded, and adds parameters such as ideal bond length
# and force constant. Note that bonds and impropers are explicitly listed
# in the CHARMM topology file, while angles and dihedrals are generated
# automatically from an existing set of bonds. These particles only define the
# bonds, but do not score them or exclude them from the nonbonded list.
bonds = topology.add_bonds(prot, ff)
angles = ff.create_angles(bonds)
dihedrals = ff.create_dihedrals(bonds)
impropers = topology.add_impropers(prot, ff)

# Maintain stereochemistry by scoring bonds, angles, dihedrals and impropers

# Score all of the bonds. This is done by combining IMP 'building blocks':
# - A ListSingletonContainer simply manages a list of the bond particles.
# - A BondSingletonScore, when given a bond particle, scores the bond by
#   calculating the distance between the two atoms it bonds, subtracting the
#   ideal value, and weighting the result by the bond's "stiffness", such that
#   an "ideal" bond scores zero, and bonds away from equilibrium score non-zero.
#   It then hands off to a UnaryFunction to actually penalize the value. In
#   this case, a Harmonic UnaryFunction is used with a mean of zero, so that
#   bond lengths are harmonically restrained.
# - A SingletonsRestraint simply goes through each of the bonds in the
#   container and scores each one in turn.
cont = IMP.container.ListSingletonContainer(bonds, "bonds")
bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))

# Score angles, dihedrals, and impropers. In the CHARMM forcefield, angles and
# impropers are harmonically restrained, so this is the same as for bonds.
# Dihedrals are scored internally by a periodic (cosine) function.
cont = IMP.container.ListSingletonContainer(angles, "angles")
bss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0,1))
m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))

cont = IMP.container.ListSingletonContainer(dihedrals, "dihedrals")
bss = IMP.atom.DihedralSingletonScore()
m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))

cont = IMP.container.ListSingletonContainer(impropers, "impropers")
bss = IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0,1))
m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))

# Add non-bonded interaction (in this case, Lennard-Jones). This needs to
# know the radii and well depths for each atom, so add them from the forcefield
# (they can also be assigned manually using the XYZR or LennardJones
# decorators):
ff.add_radii(prot)
ff.add_well_depths(prot)

# Get a list of all atoms in the protein, and put it in a container
atoms = IMP.atom.get_by_type(prot, IMP.atom.ATOM_TYPE)
cont = IMP.container.ListSingletonContainer(atoms)

# Add a restraint for the Lennard-Jones interaction. Again, this is built from
# a collection of building blocks. First, a ClosePairContainer maintains a list
# of all pairs of Particles that are close. A StereochemistryPairFilter is used
# to exclude atoms from this list that are bonded to each other or are involved
# in an angle or dihedral (1-3 or 1-4 interaction). Then, a
# LennardJonesPairScore scores a pair of atoms with the Lennard-Jones potential.
# Finally, a PairsRestraint is used which simply applies the
# LennardJonesPairScore to each pair in the ClosePairContainer.
nbl = IMP.container.ClosePairContainer(cont, 4.0)
pair_filter = IMP.atom.StereochemistryPairFilter()
pair_filter.set_bonds(bonds)
pair_filter.set_angles(angles)
pair_filter.set_dihedrals(dihedrals)
nbl.add_pair_filter(pair_filter)

sf = IMP.atom.ForceSwitch(6.0, 7.0)
ps = IMP.atom.LennardJonesPairScore(sf)
m.add_restraint(IMP.container.PairsRestraint(ps, nbl))

# Finally, evaluate the score of the whole system (without derivatives)
print m.evaluate(False)
