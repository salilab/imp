from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.kinematics

class Test(IMP.test.TestCase):
    # Failing test for manually moving joints using ProteinKinematics
    @IMP.test.expectedFailure
    def test_protein_kinematics(self):
        m = IMP.Model()
        hier = IMP.atom.read_pdb(self.open_input_file("three.pdb"), m)
        pk = IMP.kinematics.ProteinKinematics(hier)
        atoms = IMP.atom.get_by_type(hier, IMP.atom.ATOM_TYPE)

        joints = pk.get_joints()

        self.assertEqual(len(joints), 4)

        parts = IMP.atom.Selection(hier, residue_index=2).get_selected_particles()
        ats = [parts[0].get_index(), parts[1].get_index(), parts[3].get_index(), parts[4].get_index()]
        piq = IMP.ParticleIndexQuads([ats])
        residues = IMP.atom.Residues(IMP.atom.get_by_type(hier, IMP.atom.RESIDUE_TYPE))

        pk2 = IMP.kinematics.ProteinKinematics(hier, residues, piq)
        joints2 = pk2.get_joints()

        self.assertEqual(len(joints), len(joints2)-1)


        '''
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
            dd = IMP.kinematics.DOF(j.get_angle(), -1*pi, pi, pi/ang)
            dofs.append(dd)

        dd = IMP.kinematics.DirectionalDOF(dofs)
        planner = IMP.kinematics.PathLocalPlanner(mdl, sampler, dd, 10)

        rrt = IMP.kinematics.RRT(mdl, sampler, planner, dofs)
        rrt.set_scoring_function(pr)

        rrt.run()
        dof_values = rrt.get_DOFValues()
        '''

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
