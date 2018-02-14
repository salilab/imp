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
        
        # Get initial coordinates for a particle
        p1 = IMP.atom.Selection(hier, residue_index=2, atom_type=IMP.atom.AT_CB).get_selected_particles()[0]
        c1 = IMP.core.XYZ(p1).get_coordinates()
        
        # Change the joint angles
        for j in joints:
            j.set_angle(j.get_angle()+1)        
        
        # This should update cartesian coordinates
        kfss.do_before_evaluate()
        
        # See if the coordinates have changed
        c2 = IMP.core.XYZ(p1).get_coordinates()
        print(c1, c2)
        self.assertTrue(abs(c1[0]-c2[0]) > 0.0001)
        self.assertTrue(abs(c1[1]-c2[1]) > 0.0001)    
        self.assertTrue(abs(c1[2]-c2[2]) > 0.0001)
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


    def test_list_of_atoms_input(self):
        # Test to debug passing a list of list of atoms
        # to C++ functions.

        m = IMP.Model()
        hier = IMP.atom.read_pdb(self.open_input_file("three.pdb"), m)

        atoms = IMP.atom.Selection(hier, residue_index=2).get_selected_particles()

        self.assertEqual(7, len(atoms))

        #Make a list of atoms
        list_of_atoms = IMP.atom.Atoms([IMP.atom.Atom(atoms[0]), IMP.atom.Atom(atoms[1]),
                                        IMP.atom.Atom(atoms[3]), IMP.atom.Atom(atoms[4])])

        # Pass a list of lists of atoms
        n_dih = IMP.kinematics.test_atoms_list([list_of_atoms])

        self.assertEqual(n_dih, 1)

        n_dih = IMP.kinematics.test_atoms_list([list_of_atoms, list_of_atoms])

        self.assertEqual(n_dih, 2)


if __name__ == '__main__':
    IMP.test.main()
