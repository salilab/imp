## \example AA/AS_enumerate.py
''' Simple dipeptide example of ProteinKinematics including chi dihedral angle
'''

from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.kinematics
import IMP.algebra
import IMP.atom
import os
import csv

enum = True
sequence = 'AS'   # Protein sequence.  Here, two residues = 2 backbone + 1 sidechain dihedral
constant = -10    # Constant restraint used to make scoring function negative
scale = 0.55      # Radii scaling for excluded volume - between 0.5 and 0.6
pi = -3.14159256
outdir = sequence +"_enumerate/"

class ConstantRestraint(IMP.Restraint):
    """An example restraint written in Python.
       This should be functionally equivalent to the C++ ExampleRestraint.
    """
    def __init__(self, m, p, k):
        IMP.Restraint.__init__(self, m, "PythonExampleRestraint%1%")
        self.k = k
        self.p = p

    def do_add_score_and_derivatives(self, sa):
        d = IMP.core.XYZ(self.get_model(), self.p)
        score = self.k
        if sa.get_derivative_accumulator():
            deriv = 0
            d.add_to_derivative(2, deriv, sa.get_derivative_accumulator())
        sa.add_score(score)

    def do_get_inputs(self):
        return [self.get_model().get_particle(self.p)]

def scale_radii(particles, scale):
    for i in range(len(particles)):
        xyzr = IMP.core.XYZR(particles[i])
        xyzr.set_radius(xyzr.get_radius() * scale)

# Begin with Model
m = IMP.Model()

# Setup CHARMM FF
ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
topology = IMP.atom.CHARMMTopology(ff)

# Create model from this sequence
topology.add_sequence(sequence)
topology.apply_default_patches()
hier = topology.create_hierarchy(m)
topology.add_atom_types(hier)
topology.add_coordinates(hier)
ff.add_radii(hier)
ff.add_well_depths(hier)
# Setup CHARMM Force Field restraint
bonds = topology.add_bonds(hier)
angles = ff.create_angles(bonds)
dihedrals = ff.create_dihedrals(bonds)
print("# bonds ", len(bonds), " # angles ", len(angles),
        " # dihedrals ", len(dihedrals))

cont = IMP.container.ListSingletonContainer(m, bonds, "bonds")
bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
r = IMP.container.SingletonsRestraint(bss, cont, "bonds")
rs = [r]

cont = IMP.container.ListSingletonContainer(m, angles, "angles")
bss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0, 1))
r = IMP.container.SingletonsRestraint(bss, cont, "angles")
rs.append(r)

cont = IMP.container.ListSingletonContainer(m, dihedrals, "dihedrals")
bss = IMP.atom.DihedralSingletonScore()
r = IMP.container.SingletonsRestraint(bss, cont, "dihedrals")
rs.append(r)

# prepare exclusions list
pair_filter = IMP.atom.StereochemistryPairFilter()
pair_filter.set_bonds(bonds)
pair_filter.set_angles(angles)
pair_filter.set_dihedrals(dihedrals)

# add radius
atoms = IMP.atom.get_by_type(hier, IMP.atom.ATOM_TYPE)
scale_radii(atoms, scale)
cont = IMP.container.ListSingletonContainer(m, atoms)
nbl = IMP.container.ClosePairContainer(cont, 4.0)
nbl.add_pair_filter(pair_filter)
sf2 = IMP.atom.ForceSwitch(6.0, 7.0)
ps = IMP.atom.LennardJonesPairScore(sf2)
rs.append(IMP.container.PairsRestraint(ps, nbl))

# Add close pair container scores
lsc = IMP.container.ListSingletonContainer(m, IMP.get_indexes(atoms))
cpc = IMP.container.ClosePairContainer(lsc, 10.0)
cpc.add_pair_filter(pair_filter)
score = IMP.core.SoftSpherePairScore(1)
pr = IMP.container.PairsRestraint(score, cpc)
rs.append(pr)

part = IMP.atom.Selection(hier, residue_index=1, atom_type=IMP.atom.AT_CA).get_selected_particles()

const = ConstantRestraint(m, part[0], constant)
rs.append(const)
sf = IMP.core.RestraintsScoringFunction(rs)

# Build Protein Kinematics
pk = IMP.kinematics.ProteinKinematics(hier, True, True)
kf = pk.get_kinematic_forest()
joints = pk.get_joints()

# manually rotate joints
if enum:
    d = 50
    outfile = outdir + "enumerate_"+str(d)+".dat"
    print("Beginning enumeration of dihedral angles:")
    f = open(outfile, "w")
    for i in range(d):
        ang1 = i*2*pi/d
        joints[0].set_angle(ang1)
        for j in range(d):
            ang2 = j*2*pi/d
            joints[1].set_angle(ang2)
            for k in range(d):
                ang3 = k*2*pi/d
                joints[2].set_angle(ang3)

                kf.update_all_external_coordinates()

                f.write(">> " + str(sf.evaluate(False)) + " " + str(ang1) + " " + str(ang2) + " " + str(ang3) + " " + str([r.evaluate(False) for r in rs]) + "\n")

exit()
