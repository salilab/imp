import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra
import math

class ConeTests(IMP.test.TestCase):

    def test_cone_construction(self):
        """Check that cones on Z are constructed correctly"""
        s = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0,0.0,0.0),
                                  IMP.algebra.Vector3D(0.0,0.0,5.0))
        cone = IMP.algebra.Cone3D(s,4.0)
        self.assertEqual((cone.get_tip()-s.get_point(0)).get_magnitude() < 0.01,True)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(0.0,0.0,3.0)),True)
        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(0.5,0.5,3.0)),True)

        self.assertEqual(cone.get_contains(IMP.algebra.Vector3D(1.0,1.0,-3.0)),False)

    def test_sphere_patch(self):
        s = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0,0.0,0.0),
                                  IMP.algebra.Vector3D(0.0,0.0,5.0))
        cone = IMP.algebra.Cone3D(s,4.0)
        cone_pln = cone.get_intersecting_plane()
        cone_sphere = cone.get_bounding_sphere()
        sp = IMP.algebra.Sphere3DPatch(cone_sphere,cone_pln)
        for v in IMP.algebra.uniform_cover(sp,3):
            self.assertEqual(cone.get_bounding_sphere().get_contains(v),True)

    def test_sphere_patch2(self):
        s = IMP.algebra.Segment3D(IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(0.0,5.0,8.0),4.0),
                                  IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(7.0,1.0,3.0),5.0))
        cone = IMP.algebra.Cone3D(s,4.0)
        cone_pln = cone.get_intersecting_plane()
        cone_sphere = cone.get_bounding_sphere()
        sp = IMP.algebra.Sphere3DPatch(cone_sphere,cone_pln)
        for v in IMP.algebra.uniform_cover(sp,3):
            self.assertEqual(cone.get_bounding_sphere().get_contains(v),True)

if __name__ == '__main__':
    unittest.main()
