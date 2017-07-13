from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.kinematics

class Test(IMP.test.TestCase):
    # Failing test for importing ProteinKinematics class
    @IMP.test.expectedFailure
    def test_protein_kinematics(self):
        m = IMP.Model()
        hier = IMP.atom.read_pdb(self.open_input_file("three.pdb"), m)
        pk = IMP.kinematics.ProteinKinematics(hier)

        joints = pk.get_joints()
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

if __name__ == '__main__':
    IMP.test.main()
