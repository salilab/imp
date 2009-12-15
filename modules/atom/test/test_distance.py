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
                              m, IMP.atom.NonWaterSelector())
        mp1= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterSelector())
        xyz=IMP.core.XYZs(IMP.atom.get_leaves(mp))
        xyz1=IMP.core.XYZs(IMP.atom.get_leaves(mp1))
        #create a random transformation
        t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                       IMP.algebra.random_vector_in_unit_box())
        IMP.core.transform(xyz1,t)
        da=IMP.atom.placement_score(xyz1,xyz)
        d=t.get_translation().get_magnitude()
        a=IMP.algebra.decompose_rotation_into_axis_angle(t.get_rotation()).second
        self.assertAlmostEqual(da[0],d, 2)
        self.assertAlmostEqual(da[1],a, 2)

if __name__ == '__main__':
    unittest.main()
