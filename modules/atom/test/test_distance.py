import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom



class DistanceTest(IMP.test.TestCase):
    def test_placement_score(self):
        """Testing the mass and volume estimates"""
        m = IMP.Model()
        # read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        mp1= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        xyz=IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1=IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        #create a random transformation
        t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                       IMP.algebra.random_vector_in_unit_box())
        for d in xyz1: IMP.core.transform(d,t)
        da=IMP.atom.placement_score(xyz1,xyz)
        d=t.get_translation().get_magnitude()
        a=IMP.algebra.decompose_rotation_into_axis_angle(t.get_rotation()).second
        self.assertAlmostEqual(da[0],d, 2)
        self.assertAlmostEqual(da[1],a, 2)


    def test_component_placement_score(self):
        """Testing that component placement score returns the same transformation if called twice"""
        m = IMP.Model()
        # read PDB
        mp1_ref= IMP.atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        mp1_mdl= IMP.atom.read_pdb(self.open_input_file("1z5s_A.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        mp2_ref= IMP.atom.read_pdb(self.open_input_file("1z5s_C.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        mp2_mdl= IMP.atom.read_pdb(self.open_input_file("1z5s_C.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        xyz1_ref=IMP.core.XYZs(IMP.atom.get_leaves(mp1_ref))
        xyz1_mdl=IMP.core.XYZs(IMP.atom.get_leaves(mp1_mdl))
        xyz2_ref=IMP.core.XYZs(IMP.atom.get_leaves(mp2_ref))
        xyz2_mdl=IMP.core.XYZs(IMP.atom.get_leaves(mp2_mdl))

        #create a random transformation
        t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                       IMP.algebra.random_vector_in_unit_box())
        for d in xyz1_mdl: IMP.core.transform(d,t)
        t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                       IMP.algebra.random_vector_in_unit_box())
        #IMP.core.transform(xyz2_mdl,t)
        for d in xyz2_mdl: IMP.core.transform(d, t)
        da1=IMP.atom.component_placement_score(xyz1_ref,xyz2_ref,xyz1_mdl,xyz2_mdl)
        da2=IMP.atom.component_placement_score(xyz1_ref,xyz2_ref,xyz1_mdl,xyz2_mdl)
        self.assertAlmostEqual(da1[0],da2[0])
        self.assertAlmostEqual(da1[1],da2[1])


if __name__ == '__main__':
    unittest.main()
