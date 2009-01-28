import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra

class InverseTests(IMP.test.TestCase):
    """Test rigid transformations"""

    def setUp(self):
        """Build a set of test vectors"""
        IMP.test.TestCase.setUp(self)
        self.v1 = IMP.algebra.Vector3D(-63.537,76.945,84.162)
        self.v2 = IMP.algebra.Vector3D(-41.472,8.922,-63.657)


    def test_inverse(self):
        """Check that the the inverse of a transformation works"""
        rt = IMP.algebra.rotation_from_fixed_xyz(0.2,0.8,-0.4)
        t=IMP.algebra.Transformation3D(rt,IMP.algebra.Vector3D(20.0,-12.4,18.6))
        v1_t = t.transform(self.v1)
        v2_t = t.transform(self.v2)
        t_inv=t.get_inverse()
        v1_t_inv = t_inv.transform(v1_t)
        v2_t_inv = t_inv.transform(v2_t)
        self.assertEqual((v1_t_inv- self.v1).get_magnitude() < 0.01,True)
        self.assertEqual((v2_t_inv- self.v2).get_magnitude() < 0.01,True)

if __name__ == '__main__':
    unittest.main()
