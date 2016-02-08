import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.kinematics

from math import *


class RevoluteJointMoverTest(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        self.prot = IMP.atom.read_pdb(self.get_input_file_name('three.pdb'),
                self.m, IMP.atom.NonWaterNonHydrogenPDBSelector())
        #build two groups of atoms, cut at phi of residue 2
        self.residues = IMP.atom.get_by_type(self.prot, IMP.atom.RESIDUE_TYPE)
        self.residues = [IMP.atom.Residue(i) for i in self.residues]
        self.res_idx = [i.get_index() for i in self.residues]
        self.left_atoms = set()
        self.right_atoms = set()
        for i in IMP.atom.get_by_type(self.prot, IMP.atom.ATOM_TYPE):
            idx = IMP.atom.Residue(i.get_parent()).get_index()
            if idx == 1:
                self.left_atoms.add(i.get_particle())
            elif idx == 3:
                self.right_atoms.add(i.get_particle())
            else:
                at_t = IMP.atom.Atom(i).get_atom_type()
                if at_t == IMP.atom.AT_N:
                    self.left_atoms.add(i.get_particle())
                else:
                    self.right_atoms.add(i.get_particle())
        #build rbs from these groups
        self.rbs = [IMP.core.RigidBody.setup_particle(IMP.Particle(self.m),
                                                      list(self.left_atoms)),
                    IMP.core.RigidBody.setup_particle(IMP.Particle(self.m),
                                                      list(self.right_atoms))]
        # build the joint between them
        self.phi = IMP.atom.get_phi_dihedral_atoms(
                                        self.residues[self.res_idx.index(2)])
        self.phi = [IMP.core.XYZ(i.get_particle()) for i in self.phi]
        self.joint = IMP.kinematics.DihedralAngleRevoluteJoint(self.rbs[0],
                self.rbs[1], *self.phi)
        #create mover
        self.mv = IMP.kinematics.RevoluteJointMover(self.m, [self.joint], 0.1)

    def test_propose(self):
        old = self.joint.get_angle()
        self.mv.propose()
        new = self.joint.get_angle()
        self.assertTrue(abs(old - new) > 1e-7)

    def test_reject(self):
        old = self.joint.get_angle()
        self.mv.propose()
        self.mv.reject()
        new = self.joint.get_angle()
        self.assertAlmostEqual(new, old)

if __name__ == '__main__':
    IMP.test.main()
