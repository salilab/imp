import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.core

class RigidTransformationTests(IMP.test.TestCase):
    """Test particles"""

    def setUp(self):
        """Build a set of test particles"""
        IMP.test.TestCase.setUp(self)
        self.v1 = IMP.Vector3D(-63.537,76.945,84.162)
        self.v2 = IMP.Vector3D(-41.472,8.922,-63.657)


    def test_transformation(self):
        """Check that the rotation function is ok"""
        rt = IMP.core.rotation_from_fixed_xyz(0.2,0.8,-0.4)
        t=IMP.core.Transformation3D(rt,IMP.Vector3D(20.0,-12.4,18.6))
        v1_t = t.transform(self.v1)
        v2_t = t.transform(self.v2)
        v1_t_res=IMP.Vector3D(-62.517,86.209, 41.139)
        v2_t_res=IMP.Vector3D( 41.767, 1.621,-53.381)
        self.assertEqual((v1_t- v1_t_res).get_magnitude() < 0.01,True)
        self.assertEqual((v2_t- v2_t_res).get_magnitude() < 0.01,True)

    def test_center_rotation(self):
        """Check that rotation around a center is correct"""
        c= IMP.Vector3D(50,50,50)
        r= IMP.core.rotation_from_matrix(0, -1, 0,
                                         1, 0, 0,
                                         0 ,0, 1)
        tr= IMP.core.transformation_from_rotation_around_vector(r, c)
        v= IMP.Vector3D(51,50,50)
        ct= tr.transform(c)
        self.assertInTolerance(ct[0], c[0], .01)
        self.assertInTolerance(ct[1], c[1], .01)
        self.assertInTolerance(ct[2], c[2], .01)
        vt= tr.transform(v)
        self.assertInTolerance(vt[0], 50, .01)
        self.assertInTolerance(vt[1], 51, .01)
        self.assertInTolerance(vt[2], 50, .01)

if __name__ == '__main__':
    unittest.main()
