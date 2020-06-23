## \example kinematics/AA_charmm_test.py.py
"""Example constructing kinematic joints on Alanine dipeptide using ProteinKinematics,
manual manipulation of dihedral angles and scoring with the CHARMM forcefield
"""
from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.kinematics
import IMP.algebra
import IMP.atom
import IMP.rmf
import RMF
import os
import csv

pi = -3.14159256
outdir = "AA_charmm_out"

m = IMP.Model()
alad = IMP.atom.ResidueType('ALAD')
# Read in the CHARMM heavy atom topoLADogy and parameter files
ff = IMP.atom.CHARMMParameters("./data/top_all36_prot.rtf",
                                "./data/par_all36_prot.prm")
#ff = IMP.atom.get_all_atom_CHARMM_parameters()
topology = IMP.atom.CHARMMTopology(ff)

res = ff.get_residue_topology(IMP.atom.ResidueType('ALAD'))
segment = IMP.atom.CHARMMSegmentTopology()
topology.add_segment(segment)

segment.add_residue(IMP.atom.CHARMMResidueTopology(res))

hier = topology.create_hierarchy(m)
rmf = RMF.create_rmf_file(outdir+"/ala_dipeptide_grid.rmf")

# Each atom is mapped to its CHARMM type. These are needed to look up bond
# lengths, Lennard-Jones radii etc. in the CHARMM parameter file. Atom types
# can also be manually assigned at this point using the CHARMMAtom decorator.
topology.add_atom_types(hier)

# Generate and return lists of bonds, angles, dihedrals and impropers for
# the protein. Each is a Particle in the model which defines the 2, 3 or 4
# atoms that are bonded, and adds parameters such as ideal bond length
# and force constant. Note that bonds and impropers are explicitly listed
# in the CHARMM topology file, while angles and dihedrals are generated
# automatically from an existing set of bonds. These particles only define the
# bonds, but do not score them or exclude them from the nonbonded list.
bonds = topology.add_bonds(hier)
angles = ff.create_angles(bonds)
dihedrals = ff.create_dihedrals(bonds)
impropers = topology.add_impropers(hier)
topology.add_coordinates(hier)
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
cont = IMP.container.ListSingletonContainer(m, bonds, "bonds")
bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
br = IMP.container.SingletonsRestraint(bss, cont, "bonds")
rs = [br]

# Score angles, dihedrals, and impropers. In the CHARMM forcefield, angles and
# impropers are harmonically restrained, so this is the same as for bonds.
# Dihedrals are scored internally by a periodic (cosine) function.
cont = IMP.container.ListSingletonContainer(m, angles, "angles")
bss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0, 1))
ar = IMP.container.SingletonsRestraint(bss, cont, "angles")
rs.append(ar)

cont = IMP.container.ListSingletonContainer(m, dihedrals, "dihedrals")
bss = IMP.atom.DihedralSingletonScore()
dr = IMP.container.SingletonsRestraint(bss, cont, "dihedrals")
rs.append(dr)

cont = IMP.container.ListSingletonContainer(m, impropers, "impropers")
bss = IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0, 1))
rs.append(IMP.container.SingletonsRestraint(bss, cont, "impropers"))

# Add non-bonded interaction (in this case, Lennard-Jones). This needs to
# know the radii and well depths for each atom, so add them from the forcefield
# (they can also be assigned manually using the XYZR or LennardJones
# decorators):
ff.add_radii(hier)
ff.add_well_depths(hier)
IMP.rmf.add_hierarchy(rmf, hier)
# Get a list of all atoms in the protein, and put it in a container
atoms = IMP.atom.get_by_type(hier, IMP.atom.ATOM_TYPE)
cont = IMP.container.ListSingletonContainer(m, atoms)

# Add a restraint for the Lennard-Jones interaction. Again, this is built from
# a collection of building blocks. First, a ClosePairContainer maintains a list
# of all pairs of Particles that are close. A StereochemistryPairFilter is used
# to exclude atoms from this list that are bonded to each other or are involved
# in an angle or dihedral (1-3 or 1-4 interaction). Then, a
# LennardJonesPairScore scores a pair of atoms with the Lennard-Jones potential.
# Finally, a PairsRestraint is used which simply applies the
# LennardJonesPairScore to each pair in the ClosePairContainer.
nbl = IMP.container.ClosePairContainer(cont, 5.0)
pair_filter = IMP.atom.StereochemistryPairFilter()
pair_filter.set_bonds(bonds)
pair_filter.set_angles(angles)
pair_filter.set_dihedrals(dihedrals)
nbl.add_pair_filter(pair_filter)

sf = IMP.atom.ForceSwitch(6.0, 7.0)
ps = IMP.atom.LennardJonesPairScore(sf)
ljr = IMP.container.PairsRestraint(ps, nbl)
rs.append(ljr)

sf = IMP.core.RestraintsScoringFunction(rs)

charmm = IMP.atom.CHARMMStereochemistryRestraint(hier, topology)
s = IMP.core.ConjugateGradients(m)
s.set_scoring_function(sf)
s.set_log_level(IMP.TERSE)
s.optimize(1000)
IMP.atom.write_pdb(hier, outdir+"/adipep.pdb")

# it gets awfully slow with internal checks
IMP.set_check_level(IMP.USAGE)

# Finally, evaluate the score of the whole system (without derivatives)
print("INIT SCORE:", sf.evaluate(False))

# Dihedrals are: CLP-NL-CA-CRP, NL-CA-CRP-NR
# Manually annotated here
dih1 = [IMP.atom.Atom(m.get_particle(IMP.ParticleIndex(i))) for i in [7, 9, 11, 17]]
dih2 = [IMP.atom.Atom(m.get_particle(IMP.ParticleIndex(i))) for i in [9, 11, 17, 19]]

# Set up kinematic machinery
pk = IMP.kinematics.ProteinKinematics(hier, [IMP.atom.Residue(hier.get_children()[0].get_children()[0])], [dih1, dih2])
kf = pk.get_kinematic_forest()
joints = pk.get_joints()

# Make DOF list
dofs = []
for j in joints:
    dd = IMP.kinematics.DOF(j.get_angle(), -1*pi, pi, pi/50)
    dofs.append(dd)

ps = IMP.atom.Selection(hier, atom_type=IMP.atom.AT_CB).get_selected_particles()

# Initial minimization
s = IMP.core.ConjugateGradients(m)
s.set_scoring_function(sf)
s.set_log_level(IMP.TERSE)
s.optimize(100)

f = open(outdir+"/enumerate_phi_psi.dat", "w")

f.writelines(">> SF psi1 phi2 dih ljr charmm \n")

steps = 30

# Create second hierarchy to use for aligning output structures file.
m2 = IMP.Model()
pdb_hier = IMP.atom.read_pdb("./data/input.pdb", m2)
pdb_atoms = IMP.atom.get_by_type(hier, IMP.atom.ATOM_TYPE)

sel2 = [IMP.core.XYZ(p).get_coordinates() for p in IMP.atom.Selection([IMP.atom.Hierarchy(m2.get_particle(IMP.ParticleIndex(i))) for i in [9, 11, 17]]).get_selected_particles()]

s = IMP.core.ConjugateGradients(m)
s2 = IMP.core.SteepestDescent(m)

# Do enumeration of dihedral angles
for i in range(steps):
    print(i, "of", steps, "steps")
    ang1 = -pi + i*2*pi/steps
    joints[0].set_angle(ang1)
    kf.update_all_external_coordinates()
    for j in range(steps):
        ang2 = -pi + j*2*pi/steps
        joints[1].set_angle(ang2)

        m.update()

        func1 = IMP.core.Harmonic(ang1,100000)
        func2 = IMP.core.Harmonic(ang2,100000)
        dihr1 = IMP.core.DihedralRestraint(m, func1, IMP.ParticleIndex(7), IMP.ParticleIndex(9), IMP.ParticleIndex(11), IMP.ParticleIndex(17))
        dihr2 = IMP.core.DihedralRestraint(m, func2, IMP.ParticleIndex(9), IMP.ParticleIndex(11), IMP.ParticleIndex(17), IMP.ParticleIndex(19))
        sf1 = IMP.core.RestraintsScoringFunction(rs + [dihr1, dihr2])
        # Minimize to the dihedrals provides
        s.set_scoring_function(sf1)
        s2.set_scoring_function(sf1)
        s2.optimize(1000)
        s.optimize(1000)

        sel1 = [IMP.core.XYZ(p).get_coordinates() for p in IMP.atom.Selection([IMP.atom.Hierarchy(m.get_particle(IMP.ParticleIndex(i))) for i in [9, 11, 17]]).get_selected_particles()]
        t3d = IMP.algebra.get_transformation_aligning_first_to_second(sel1, sel2)

        IMP.atom.transform(hier, t3d)

        for p in atoms:
            IMP.core.transform(IMP.core.XYZ(p), t3d)
        IMP.rmf.save_frame(rmf)
        f.writelines(">> " + str(sf.evaluate(False)) + " " + str(ang1) + " " + str(ang2) + " " + str(dr.evaluate(False)) + " " + str(ljr.evaluate(False)) + " " + str(charmm.evaluate(False)) + "\n")
