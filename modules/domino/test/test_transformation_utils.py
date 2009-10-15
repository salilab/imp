import sys
import unittest
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.helper
import time

class DOMINOTests(IMP.test.TestCase):

    def __set_rigid_bodies__(self):
        self.rbs = IMP.Particles()
        names=["A","B","C","D"]
        for i,mh in enumerate(self.mhs):
            rb_p = IMP.Particle(self.mdl)
            rb_state = IMP.helper.create_rigid_body(rb_p,
                            IMP.core.XYZs(IMP.core.get_leaves(mh)))
            self.mdl.add_score_state(rb_state)
            #make sure the rigid body has a large enough radius
            #to include their members
            self.rbs.append(rb_p)

    def __set_components__(self):
        self.mhs = IMP.atom.Hierarchies()
        sel = IMP.atom.CAlphaSelector()
        for fn,name in [["1z5s_A.pdb","A"],["1z5s_B.pdb","B"],
                   ["1z5s_C.pdb","C"],["1z5s_D.pdb","D"]]:
            self.mhs.append(IMP.atom.read_pdb(self.open_input_file(fn),
                                              self.mdl,sel))
            IMP.atom.add_radii(self.mhs[-1])


    def __set_components_copy__(self):
        self.mhs_copy = IMP.atom.Hierarchies()
        sel = IMP.atom.CAlphaSelector()
        for fn,name in [["1z5s_A.pdb","A"],["1z5s_B.pdb","B"],
                   ["1z5s_C.pdb","C"],["1z5s_D.pdb","D"]]:
            self.mhs_copy.append(IMP.atom.read_pdb(self.open_input_file(fn),
                                              self.mdl,sel))
            IMP.atom.add_radii(self.mhs_copy[-1])


    def __set_discrete_sampling_space__(self):
        self.set_transformation_attributes()
        self.discrete_set =IMP.domino.TransformationMappedDiscreteSet(self.rbs)
        zero_vec=IMP.algebra.Vector3D(0.,0.,0.)
        self.states_for_rb=3
        for i,rb in enumerate(self.rbs):
            i_t=IMP.algebra.identity_transformation()
            state_p = self.create_transformation_as_particle(i_t)
            self.discrete_set.add_mapped_state(rb,state_p)
            for j in range(self.states_for_rb):
                rand_t=IMP.algebra.Transformation3D(
                    IMP.algebra.random_rotation(),
                    IMP.algebra.random_vector_in_sphere(zero_vec,4.0))

                state_p = self.create_transformation_as_particle(rand_t)
                self.discrete_set.add_mapped_state(rb,state_p)
        self.sampler = IMP.domino.TransformationCartesianProductSampler(self.discrete_set,self.rbs,True)


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
        for j in range(5):
            for i in range(4):
                #generate random transformation
                rand_t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                                    IMP.algebra.random_vector_in_unit_box())
                #transform the copy molecule
                xyz_copy=IMP.core.XYZs(IMP.core.get_leaves(self.mhs_copy[i]))
                xyz_orig=IMP.core.XYZs(IMP.core.get_leaves(self.mhs[i]))
                for xyz in xyz_copy:
                    xyz.set_coordinates(rand_t.transform(xyz.get_coordinates()))
                #transform the rigid body
                rand_t_p = IMP.domino.Transformation.setup_particle(IMP.Particle(self.mdl),rand_t)
                self.tu.move2state(self.rbs[i],rand_t_p.get_particle())
                self.mdl.evaluate(None) #to make sure that the rigid bodies score states are updated
                #check that the rmsd is 0
                self.assert_( IMP.atom.rmsd(xyz_copy,xyz_orig) < 0.001,
                           "the molecules are expected to have the same placement")
                #return the copy to ref for the next round
                for xyz in xyz_copy:
                    xyz.set_coordinates(rand_t.get_inverse().transform(xyz.get_coordinates()))

if __name__ == '__main__':
    unittest.main()
