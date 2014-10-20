import IMP
import IMP.test
import IMP.algebra
import math
from StringIO import StringIO

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
        bb = IMP.algebra.get_bounding_box(patch)
        self.assertLess(IMP.algebra.get_distance(bb.get_corner(0),
                                     IMP.algebra.Vector3D(-5,-5,-5)), 1e-4)
        self.assertLess(IMP.algebra.get_distance(bb.get_corner(1),
                                     IMP.algebra.Vector3D(5,5,5)), 1e-4)
        p = patch.get_boundary_point()
        self.assertLess(IMP.algebra.get_distance(p,
                              IMP.algebra.Vector3D(3.53553, 3.53553, 0)), 1e-3)
        sio = StringIO()
        patch.show(sio)
        self.assertEqual(sio.getvalue(), '(0 0 0: 5)(0: 0 0 1)')
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


if __name__ == '__main__':
    IMP.test.main()
