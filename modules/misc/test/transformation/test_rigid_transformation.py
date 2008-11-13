import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.misc

class RigidTransformationTests(IMP.test.TestCase):
    """Test particles"""

    def setUp(self):
        """Build a set of test particles"""
        IMP.test.TestCase.setUp(self)
        self.v1 = IMP.Vector3D(-63.537,76.945,84.162)
        self.v2 = IMP.Vector3D(-41.472,8.922,-63.657)


    def test_transformation(self):
        """Check that the rotation function is ok"""
        t=IMP.misc.Transformation3D(IMP.misc.Rotation3D(0.2,0.8,-0.4),IMP.Vector3D(20.0,-12.4,18.6))
        v1_t = t.transform(self.v1)
        v2_t = t.transform(self.v2)
        v1_t_res=IMP.Vector3D(-62.517,86.209, 41.139)
        v2_t_res=IMP.Vector3D( 41.767, 1.621,-53.381)
        self.assertEqual((v1_t- v1_t_res).get_magnitude() < 0.01,True)
        self.assertEqual((v2_t- v2_t_res).get_magnitude() < 0.01,True)

if __name__ == '__main__':
    unittest.main()
