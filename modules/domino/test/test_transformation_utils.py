import sys
import unittest
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.helper
import time
import helpers

class TransformationUtilsTests(IMP.test.TestCase):

    def __set_rigid_bodies__(self):
        self.rbs=helpers.set_rigid_bodies(self.mhs,self.mdl)

    def __set_components__(self):
        self.mhs = helpers.read_components([
            self.open_input_file("1z5s_A.pdb"),
            self.open_input_file("1z5s_B.pdb"),
            self.open_input_file("1z5s_C.pdb"),
            self.open_input_file("1z5s_D.pdb")],self.mdl)

    def __set_components_copy__(self):
        self.mhs_copy = helpers.read_components([
            self.open_input_file("1z5s_A.pdb"),
            self.open_input_file("1z5s_B.pdb"),
            self.open_input_file("1z5s_C.pdb"),
            self.open_input_file("1z5s_D.pdb")],self.mdl)

    def setUp(self):

        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.mdl = IMP.Model()
        IMP.set_check_level(IMP.NONE)
        self.__set_components__()
        self.__set_components_copy__()
        self.__set_rigid_bodies__()
        self.tu = IMP.domino.TransformationUtils(self.rbs,True)

    def test_transformation_on_rigid_bodies(self):
        '''test transformation utils on rigid bodies'''
        for j in range(5): # 5 test cases
            for i in range(4):
                #generate random transformation
                rand_t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                                    IMP.algebra.random_vector_in_unit_box())
                #transform the copy molecule
                xyz_copy=IMP.core.XYZs(IMP.core.get_leaves(self.mhs_copy[i]))
                for xyz in xyz_copy:
                    xyz.set_coordinates(rand_t.transform(xyz.get_coordinates()))
                #transform the rigid body
                xyz_orig=IMP.core.XYZs(IMP.core.get_leaves(self.mhs[i]))
                rand_t_p = IMP.domino.Transformation.setup_particle(IMP.Particle(self.mdl),rand_t)
                self.tu.move2state(self.rbs[i],rand_t_p.get_particle())
                self.mdl.evaluate(None) #to make sure that the rigid bodies score states are updated
                #check that the rmsd is 0
                self.assert_(IMP.atom.rmsd(xyz_copy,xyz_orig) < 0.001,
                           "the molecules are expected to have the same placement")
                #return the copy to ref for the next round
                for xyz in xyz_copy:
                    xyz.set_coordinates(rand_t.get_inverse().transform(xyz.get_coordinates()))


    def test_transformation_on_rigid_bodies(self):
        '''test transformation utils on rigid bodies'''
        for j in range(5): # 5 test cases
            for i in range(4):
                #generate random transformation
                rand_t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                                    IMP.algebra.random_vector_in_unit_box())
                #transform the copy molecule
                xyz_copy=IMP.core.XYZs(IMP.core.get_leaves(self.mhs_copy[i]))
                for xyz in xyz_copy:
                    xyz.set_coordinates(rand_t.transform(xyz.get_coordinates()))
                #transform the rigid body
                xyz_orig=IMP.core.XYZs(IMP.core.get_leaves(self.mhs[i]))
                rand_t_p = IMP.domino.Transformation.setup_particle(IMP.Particle(self.mdl),rand_t)
                self.tu.move2state(self.rbs[i],rand_t_p.get_particle())
                self.mdl.evaluate(None) #to make sure that the rigid bodies score states are updated
                #check that the rmsd is 0
                self.assert_(IMP.atom.rmsd(xyz_copy,xyz_orig) < 0.001,
                           "the molecules are expected to have the same placement")
                #return the copy to ref for the next round
                for xyz in xyz_copy:
                    xyz.set_coordinates(rand_t.get_inverse().transform(xyz.get_coordinates()))


    def test_transformation_on_rigid_bodies2(self):
        '''test transformation utils on rigid bodies'''
        tu = IMP.domino.TransformationUtils(self.rbs)
        for j in range(5):
            for i in range(4):
                trans = helpers.create_random_transformation()
                state_p=IMP.Particle(self.mdl)
                IMP.domino.Transformation.setup_particle(state_p,trans)

                #transform the copy molecule
                xyz_copy=IMP.core.XYZs(IMP.core.get_leaves(self.mhs_copy[i]))
                for xyz in xyz_copy:
                    xyz.set_coordinates(trans.transform(xyz.get_coordinates()))


                #transform the rigid body
                xyz_orig=IMP.core.XYZs(IMP.core.get_leaves(self.mhs[i]))
                self.tu.move2state(self.rbs[i],state_p)
            self.mdl.evaluate(None) #to make sure that the rigid bodies score states are updated
            #check that the rmsd is 0
            self.assert_(IMP.atom.rmsd(xyz_copy,xyz_orig) < 0.001,
                         "the molecules are expected to have the same placement")
            #return the copy to ref for the next round
            for xyz in xyz_copy:
                xyz.set_coordinates(trans.get_inverse().transform(xyz.get_coordinates()))






if __name__ == '__main__':
    unittest.main()
