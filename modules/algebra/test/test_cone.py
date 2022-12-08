from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import math
import pickle


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
        g = IMP.algebra.get_cone_3d_geometry(c)
        self.assertLess((g.get_tip() - c.get_tip()).get_magnitude(), 1e-4)

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
        print(cone)

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

    def test_surface_cover(self):
        """Check surface cover of cone"""
        s = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0, 0.0, 0.0),
                                  IMP.algebra.Vector3D(0.0, 0.0, 5.0))
        cone = IMP.algebra.Cone3D(s, 4.0)
        pts = IMP.algebra.get_uniform_surface_cover(cone, 10)
        self.assertEqual(len(pts), 10)
        for p in pts:
            self.assertTrue(cone.get_contains(p))

    def _assert_equal(self, a, b):
        self.assertLess(IMP.algebra.get_distance(
            a.get_tip(), b.get_tip()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            a.get_direction(), b.get_direction()), 1e-4)
        self.assertAlmostEqual(a.get_height(), b.get_height(), delta=1e-4)
        self.assertAlmostEqual(a.get_angle(), b.get_angle(), delta=1e-4)

    def test_pickle(self):
        """Test (un-)pickle of Cone3D"""
        s1 = IMP.algebra.Segment3D(IMP.algebra.Vector3D(0.0, 0.0, 0.0),
                                  IMP.algebra.Vector3D(0.0, 0.0, 5.0))
        cone1 = IMP.algebra.Cone3D(s1, 4.0)
        s2 = IMP.algebra.Segment3D(IMP.algebra.Vector3D(1.0, 2.0, 3.0),
                                  IMP.algebra.Vector3D(0.0, 0.0, 5.0))
        cone2 = IMP.algebra.Cone3D(s2, 6.0)
        cone2.foo = 'bar'
        dump = pickle.dumps((cone1, cone2))

        newcone1, newcone2 = pickle.loads(dump)
        self._assert_equal(cone1, newcone1)
        self._assert_equal(cone2, newcone2)
        self.assertEqual(newcone2.foo, 'bar')

        self.assertRaises(TypeError, cone1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
