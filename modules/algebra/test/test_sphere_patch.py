import IMP
import IMP.test
import IMP.algebra
import math
from io import BytesIO
import pickle


class Tests(IMP.test.TestCase):

    def test_trivial_constructor(self):
        """Test trivial SpherePatch3D constructor"""
        p = IMP.algebra.SpherePatch3D()

    def test_sphere_patch_construction(self):
        """Check that a patch of a sphere is constructed correctly"""
        center = IMP.algebra.Vector3D(0.0, 0.0, 0.0)
        radius = 5.0
        sph = IMP.algebra.Sphere3D(center, radius)
        xy_plane = IMP.algebra.Plane3D(
            IMP.algebra.Vector3D(0., 0., 0.), IMP.algebra.Vector3D(0., 0., 1.))
        patch = IMP.algebra.SpherePatch3D(sph, xy_plane)
        # Not implemented
        self.assertRaises(Exception, IMP.algebra.get_area, patch)
        g = IMP.algebra.get_sphere_patch_3d_geometry(patch)
        self.assertLess(IMP.algebra.get_distance(patch.get_plane().get_normal(),
                                                 g.get_plane().get_normal()),
                                                 1e-4)
        self.assertLess(IMP.algebra.get_distance(
                          patch.get_sphere().get_center(),
                          g.get_sphere().get_center()), 1e-4)
        bb = IMP.algebra.get_bounding_box(patch)
        self.assertLess(IMP.algebra.get_distance(bb.get_corner(0),
                                     IMP.algebra.Vector3D(-5,-5,-5)), 1e-4)
        self.assertLess(IMP.algebra.get_distance(bb.get_corner(1),
                                     IMP.algebra.Vector3D(5,5,5)), 1e-4)
        p = patch.get_boundary_point()
        self.assertLess(IMP.algebra.get_distance(p,
                              IMP.algebra.Vector3D(3.53553, 3.53553, 0)), 1e-3)
        sio = BytesIO()
        patch.show(sio)
        self.assertEqual(sio.getvalue(), b'(0 0 0: 5)(0: 0 0 1)')
        self.assertTrue(patch.get_plane().get_is_above(
                                 IMP.algebra.Vector3D(1., 0., 1.)))
        self.assertTrue(patch.get_contains(IMP.algebra.Vector3D(0.0, 1.0, 0.4)))
        self.assertFalse(patch.get_contains(
                                         IMP.algebra.Vector3D(0.0, 1.0, -0.4)))

    def test_get_uniform_surface_cover(self):
        """Check uniform cover on a patch of a sphere"""
        center = IMP.algebra.Vector3D(0.0, 0.0, 0.0)
        radius = 5.0
        sph = IMP.algebra.Sphere3D(center, radius)
        xz_plane = IMP.algebra.Plane3D(
            IMP.algebra.Vector3D(0., 0., 0.), IMP.algebra.Vector3D(0., 1., 0.))
        patch = IMP.algebra.SpherePatch3D(sph, xz_plane)
        numpts = 600
        points = IMP.algebra.get_uniform_surface_cover(patch, numpts)
        # check that the centroid of the sampled points make sense
        sampled_centroid = IMP.algebra.Vector3D(0.0, radius / 2, 0.0)
        expected_sampled_centroid = IMP.algebra.Vector3D(0.0, radius / 2, 0.0)
        self.assertEqual(len(points), numpts)
        for p in points:
            sampled_centroid = sampled_centroid + p
        sampled_centroid = sampled_centroid * (1.0 / len(points))
        sampled_centroid.show()
        self.assertAlmostEqual(
            (sampled_centroid - expected_sampled_centroid).get_magnitude(), 0,
            delta=4 * radius / numpts ** .5)

    def _assert_equal(self, a, b):
        spherea = a.get_sphere()
        sphereb = b.get_sphere()
        self.assertLess(IMP.algebra.get_distance(
            spherea.get_center(), sphereb.get_center()), 1e-4)
        self.assertAlmostEqual(spherea.get_radius(), sphereb.get_radius(),
                               delta=1e-4)
        planea = a.get_plane()
        planeb = b.get_plane()
        self.assertLess(IMP.algebra.get_distance(
            planea.get_normal(), planeb.get_normal()), 1e-4)
        self.assertLess(IMP.algebra.get_distance(
            planea.get_point_on_plane(), planeb.get_point_on_plane()), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of SpherePatch3D"""
        sph = IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1.0, 2.0, 3.0), 5.0)
        plane = IMP.algebra.Plane3D(
            IMP.algebra.Vector3D(0., 0., 0.), IMP.algebra.Vector3D(0., 1., 0.))
        p1 = IMP.algebra.SpherePatch3D(sph, plane)
        sph = IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4.0, 5.0, 6.0), 5.0)
        plane = IMP.algebra.Plane3D(
            IMP.algebra.Vector3D(0., 0., 0.), IMP.algebra.Vector3D(1., 0., 0.))
        p2 = IMP.algebra.SpherePatch3D(sph, plane)
        p2.foo = 'bar'
        dump = pickle.dumps((p1, p2))

        newp1, newp2 = pickle.loads(dump)
        self._assert_equal(p1, newp1)
        self._assert_equal(p2, newp2)
        self.assertEqual(newp2.foo, 'bar')

        self.assertRaises(TypeError, p1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
