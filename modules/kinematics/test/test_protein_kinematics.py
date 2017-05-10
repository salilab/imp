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

if __name__ == '__main__':
    IMP.test.main()
