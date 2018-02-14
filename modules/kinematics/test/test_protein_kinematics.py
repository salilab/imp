from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.kinematics
import IMP.algebra
import IMP.atom
#import IMP.atom.CHARMMParameters
#import IMP.atom.CHARMMStereochemistryRestraint

class Test(IMP.test.TestCase):

    def construct_peptide(self):
        topology = IMP.atom.CHARMMTopology(IMP.atom.get_all_atom_CHARMM_parameters())
        topology.add_sequence('VQIVYK')
        topology.apply_default_patches()
        m = IMP.Model()
        h = topology.create_hierarchy(m)
        topology.add_atom_types(h)
        topology.add_coordinates(h)
        IMP.atom.add_radii(h)

        self.topology = topology

        return m, h

    def scale_radii(self, particles, scale):
        for i in range(len(particles)):
            xyzr = IMP.core.XYZR(particles[i])
            xyzr.set_radius(xyzr.get_radius() * scale)


    def add_charmm_ff_restraint(self, m, hier):

        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()

        #topology = ff.create_topology(hier)
        #topology.setup_hierarchy(hier)
        bonds = self.topology.add_bonds(hier)
        angles = ff.create_angles(bonds)
        dihedrals = ff.create_dihedrals(bonds)
        print("# bonds ", len(bonds), " # angles ", len(angles),
                " # dihedrals ", len(dihedrals))

        # add radius
        #ff.add_radii(hier)
        atoms = IMP.atom.get_by_type(hier, IMP.atom.ATOM_TYPE)
        self.scale_radii(atoms, 0.4)

        # prepare exclusions list
        pair_filter = IMP.atom.StereochemistryPairFilter()
        pair_filter.set_bonds(bonds)
        pair_filter.set_angles(angles)
        pair_filter.set_dihedrals(dihedrals)

        # close pair container
        lsc = IMP.container.ListSingletonContainer(m, IMP.get_indexes(atoms))

        cpc = IMP.container.ClosePairContainer(lsc, 15.0)
        cpc.add_pair_filter(pair_filter)

        score = IMP.core.SoftSpherePairScore(1)
        pr = IMP.container.PairsRestraint(score, cpc)

        return pr

    def test_protein_kinematics(self):
        m, hier = self.construct_peptide()
        #hier = IMP.atom.read_pdb(self.open_input_file("three.pdb"), m)
        pk = IMP.kinematics.ProteinKinematics(hier)
        pi = 3.1415

        joints = pk.get_joints()

        self.assertEqual(len(joints), 10)


        # Test a construction adding a single custom dihedral on the side chain of residue 2
        parts = IMP.atom.Selection(hier, residue_index=2).get_selected_particles()
        ats = [parts[0].get_index(), parts[1].get_index(), parts[3].get_index(), parts[4].get_index()]
        piq = IMP.ParticleIndexQuads([ats])
        residues = IMP.atom.Residues(IMP.atom.get_by_type(hier, IMP.atom.RESIDUE_TYPE))

        pk2 = IMP.kinematics.ProteinKinematics(hier, residues, piq)
        joints2 = pk2.get_joints()

        # Test that one more joint was created
        self.assertEqual(len(joints), len(joints2)-1)


        # Eventually expand test through RRT steps
        atoms = IMP.atom.get_by_type(hier, IMP.atom.ATOM_TYPE)
        kfss = IMP.kinematics.KinematicForestScoreState(pk.get_kinematic_forest(), pk.get_rigid_bodies(), atoms)
        m.add_score_state(kfss)

        dofs = []

        for j in joints:
            dd = IMP.kinematics.DOF(j.get_angle(), -1*pi, pi, pi/360)
            dofs.append(dd)

        sampler = IMP.kinematics.UniformBackboneSampler(joints, dofs)

        dd = IMP.kinematics.DirectionalDOF(dofs)
        planner = IMP.kinematics.PathLocalPlanner(m, sampler, dd, 10)

        pr = self.add_charmm_ff_restraint(m, hier)

        rrt = IMP.kinematics.RRT(m, sampler, planner, dofs)
        rrt.set_scoring_function(pr)

        rrt.run()

        dof_values = rrt.get_DOFValues()

        print(len(dof_values))


if __name__ == '__main__':
    IMP.test.main()
