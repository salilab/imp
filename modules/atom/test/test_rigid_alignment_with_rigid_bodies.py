import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.helper


class Alignment_Rigid_BodiesTests(IMP.test.TestCase):
    def test_alignment_without_rigid_body(self):
        """Rigid aligment without rigid bodies"""
        mdl=IMP.Model()
        mh1=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),mdl)
        xyz1=IMP.core.XYZs(IMP.core.get_leaves(mh1))
        vec1=[]
        for xyz in xyz1:
            vec1.append(xyz.get_coordinates())
        mh2=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),mdl)
        xyz2=IMP.core.XYZs(IMP.core.get_leaves(mh2))
        #randomize position of mh2
        rand_t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                            IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        vec2=[]
        for xyz in xyz2:
            xyz.set_coordinates(rand_t.get_transformed(xyz.get_coordinates()))
            vec2.append(xyz.get_coordinates())

        #align mh1 onto mh2
        align_t=IMP.algebra.get_transformation_taking_first_to_second(vec1,vec2)
        for xyz in xyz1:
            xyz.set_coordinates(align_t.get_transformed(xyz.get_coordinates()))
        self.assertInTolerance(IMP.atom.get_rmsd(xyz1,xyz2),0.,.01)
    def test_alignment_with_rigid_body(self):
        """Rigid aligment with rigid bodies"""
        mdl=IMP.Model()
        mh1=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),mdl)
        xyz1=IMP.core.XYZs(IMP.core.get_leaves(mh1))
        vec1=[x.get_coordinates() for x in xyz1]
        mh2=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),mdl)
        xyz2=IMP.core.XYZs(IMP.core.get_leaves(mh2))
        #randomize position of mh2
        rand_t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                            IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        print rand_t
        vec2=[]
        for xyz in xyz2:
            xyz.set_coordinates(rand_t.get_transformed(xyz.get_coordinates()))
            vec2.append(xyz.get_coordinates())

        rb1=IMP.atom.setup_as_rigid_body(mh1)
        print rb1.get_reference_frame()
        #align mh1 onto mh2
        align_t=IMP.algebra.get_transformation_taking_first_to_second(vec1,vec2)
        print align_t
        #now transform mh1 accordinly
        IMP.core.transform(rb1,align_t)
        #rb1.set_reference_frame(IMP.algebra.get_transformed(rb1.get_reference_frame(),
        #                                                    align_t))
        print rb1.get_reference_frame()
        mdl.set_log_level(IMP.SILENT)
        #mdl.update()
        #we acpect a rmsd of 0
        self.assertInTolerance(IMP.atom.get_rmsd(xyz1,xyz2),0.,.01)
if __name__ == '__main__':
    unittest.main()
