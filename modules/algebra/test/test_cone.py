import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra
import IMP.display
import math
class ConeTests(IMP.test.TestCase):
    """Test rigid transformations"""

    def setUp(self):
        """Build a set of test vectors"""
        IMP.test.TestCase.setUp(self)


    def test_close_cone_construction(self):
        """Check that close cones are constructed correctly"""
        tip = IMP.algebra.Vector3D(0.0,0.0,0.0)
        direction = IMP.algebra.Vector3D(1.0,0.0,0.0)
        h=5.0;angle=math.pi/3;
        cone = IMP.algebra.SphericalCone3D(tip,direction,angle,h)
        self.assertEqual((cone.get_tip()-tip).get_magnitude() < 0.01,True)
        self.assertEqual(cone.is_bound(),True)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(0.5,0.5,0.5)),True)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(0.5,1.0,9.5)),False)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(10.0,1.0,0.5)),False)
    def test_open_cone_construction(self):
        """Check that open cones are constructed correctly"""
        tip = IMP.algebra.Vector3D(0.0,0.0,0.0)
        direction = IMP.algebra.Vector3D(1.0,0.0,0.0)
        angle=math.pi/3;
        cone = IMP.algebra.SphericalCone3D(tip,direction,angle)
        self.assertEqual((cone.get_tip()-tip).get_magnitude() < 0.01,True)
        self.assertEqual(cone.is_bound(),False)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(0.5,0.5,0.5)),True)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(0.5,1.0,9.5)),False)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(10.0,1.0,0.5)),True)
if __name__ == '__main__':
    unittest.main()
