import IMP
import IMP.test
import IMP.algebra
import math


class Tests(IMP.test.TestCase):

    def test_trivial_constructor(self):
        """Check trivial cone constructor"""
        c = IMP.algebra.Cone3D()

    def test_volume_methods(self):
        """Test namespace methods for Cone3D"""
        s = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0, 0.0, 0.0),
                                  IMP.algebra.Vector3D(0.0, 0.0, 5.0))
        c = IMP.algebra.Cone3D(s, 4.0)
        # Not implemented
        self.assertRaises(Exception, IMP.algebra.get_surface_area, c)
        self.assertRaises(Exception, IMP.algebra.get_volume, c)
        self.assertRaises(Exception, IMP.algebra.get_bounding_box, c)

    def test_cone_construction(self):
        """Check that cones on Z are constructed correctly"""
        s = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0, 0.0, 0.0),
                                  IMP.algebra.Vector3D(0.0, 0.0, 5.0))
        cone = IMP.algebra.Cone3D(s, 4.0)
        self.assertAlmostEqual(cone.get_angle(), 1.3495, delta=1e-4)
        self.assertAlmostEqual(cone.get_radius(), 4.0, delta=1e-4)
        self.assertAlmostEqual(cone.get_height(), 5.0, delta=1e-4)
        p = cone.get_base_plane()
        self.assertAlmostEqual(p.get_height(cone.get_tip()), 5.0, delta=1e-5)
        self.assertLess((cone.get_direction() - IMP.algebra.Vector3D(0,0,1)).get_magnitude(), 0.01)
        self.assertLess((cone.get_tip() - s.get_point(0)).get_magnitude(), 0.01)
        self.assertTrue(cone.get_contains(IMP.algebra.Vector3D(0.0, 0.0, 3.0)))
        self.assertTrue(cone.get_contains(IMP.algebra.Vector3D(0.5, 0.5, 3.0)))
        self.assertFalse(cone.get_contains(IMP.algebra.Vector3D(0.5, 0.5, 0.0)))
        self.assertFalse(
                  cone.get_contains(IMP.algebra.Vector3D(1.0, 1.0, -3.0)))
        print cone

    def test_sphere_patch2(self):
        """Testing sampling a patch"""
        sphere = IMP.algebra.Sphere3D(
            IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()),
            10)
        n = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
        p = IMP.algebra.get_random_vector_in(sphere)
        plane = IMP.algebra.Plane3D(p, n)
        sp = IMP.algebra.SpherePatch3D(sphere, plane)
        bs = IMP.algebra.Sphere3D(sphere.get_center(),
                                  sphere.get_radius() * 1.1)
        for v in IMP.algebra.get_uniform_surface_cover(sp, 3):
            self.assertTrue(bs.get_contains(v))

if __name__ == '__main__':
    IMP.test.main()
