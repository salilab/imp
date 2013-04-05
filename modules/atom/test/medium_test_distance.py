import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.algebra as alg
import math

class Tests(IMP.test.TestCase):

    def test_component_placement_score(self):
        """Testing that component placement score returns the same transformation if called twice"""
        m = IMP.Model()
        # read PDB
        mp1_ref= atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp1_mdl= atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp2_ref= atom.read_pdb(self.open_input_file("1z5s_C.pdb"),
                              m, atom.NonWaterPDBSelector())
        mp2_mdl= atom.read_pdb(self.open_input_file("1z5s_C.pdb"),
                              m, atom.NonWaterPDBSelector())
        xyz1_ref=core.XYZs(atom.get_leaves(mp1_ref))
        xyz1_mdl=core.XYZs(atom.get_leaves(mp1_mdl))
        xyz2_ref=core.XYZs(atom.get_leaves(mp2_ref))
        xyz2_mdl=core.XYZs(atom.get_leaves(mp2_mdl))

        #create a random transformation
        t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        for d in xyz1_mdl: core.transform(d,t)
        t=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                       IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        #core.get_transformed(xyz2_mdl,t)
        for d in xyz2_mdl: core.transform(d, t)
        da1=atom.get_component_placement_score(xyz1_ref,xyz2_ref,xyz1_mdl,xyz2_mdl)
        da2=atom.get_component_placement_score(xyz1_ref,xyz2_ref,xyz1_mdl,xyz2_mdl)
        self.assertAlmostEqual(da1[1],da2[1])


if __name__ == '__main__':
    IMP.test.main()
