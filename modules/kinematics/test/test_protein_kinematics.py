from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.kinematics

class Test(IMP.test.TestCase):
    # Failing test for importing ProteinKinematics class
    def test_protein_kinematics(self):
        m = IMP.Model()
        hier = IMP.atom.read_pdb(self.open_input_file("three.pdb"), m)
        pk = IMP.kinematics.ProteinKinematics(hier)

        joints = pk.get_joints()


        self.assertEqual(len(joints), 4)

        '''
        kfss = IMP.kinematics.KinematicForestScoreState(pk.get_kinematic_forest(), pk.get_rigid_bodies(), atoms)
        m.add_score_state(kfss)

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
