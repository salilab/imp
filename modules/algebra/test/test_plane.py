import IMP
import IMP.test
import IMP.algebra
import math
import pickle
from io import BytesIO

class Tests(IMP.test.TestCase):

    def test_trivial_constructor(self):
        """Check trivial plane constructor"""
        p = IMP.algebra.Plane3D()

    def test_area_methods(self):
        """Test namespace methods for Plane3D"""
        p = IMP.algebra.Plane3D(5.0, IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        self.assertGreater(IMP.algebra.get_area(p), 1e20) # infinite area
        bb = IMP.algebra.get_bounding_box(p)
        g = IMP.algebra.get_plane_3d_geometry(p)
        self.assertLess(IMP.algebra.get_distance(p.get_normal(),
                                                 g.get_normal()), 1e-4)

    def test_plane(self):
        """Test Plane3D methods"""
        p = IMP.algebra.Plane3D(IMP.algebra.Vector3D(0., 5., 0.),
                                IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        sio = BytesIO()
        p.show(sio)
        self.assertLess(IMP.algebra.get_distance(p.get_normal(),
                               IMP.algebra.Vector3D(0., 1., 0.)), .1)
        self.assertLess(IMP.algebra.get_distance(p.get_point_on_plane(),
                               IMP.algebra.Vector3D(0., 5., 0.)), .1)
        pr = p.get_projected(IMP.algebra.Vector3D(5., 0., 0.))
        self.assertLess(IMP.algebra.get_distance(pr,
                               IMP.algebra.Vector3D(5., 5., 0.)), .1)
        self.assertAlmostEqual(p.get_height(IMP.algebra.Vector3D(0., 14., 0.)),
                               9.0, delta=1e-4)
        self.assertTrue(p.get_is_above(IMP.algebra.Vector3D(0., 14., 0.)))
        self.assertFalse(p.get_is_below(IMP.algebra.Vector3D(0., 14., 0.)))
        self.assertAlmostEqual(p.get_distance_from_origin(), 5.0, delta=1e-4)
        opp = p.get_opposite()
        self.assertAlmostEqual(opp.get_distance_from_origin(), -5.0, delta=1e-4)
        self.assertAlmostEqual(IMP.algebra.get_distance(
                        p, IMP.algebra.Vector3D(0., 1., 40.)), 4.0, delta=1e-4)
        ref = IMP.algebra.get_reflected(p, IMP.algebra.Vector3D(0., 1., 40.))
        self.assertLess(IMP.algebra.get_distance(ref,
                                IMP.algebra.Vector3D(0., 9., 40.)), 1e-4)

    def test_pickle(self):
        """Test (un-)pickle of Plane3D"""
        p1 = IMP.algebra.Plane3D(5.0, IMP.algebra.Vector3D(0.0, 1.0, 0.0))
        p2 = IMP.algebra.Plane3D(3.0, IMP.algebra.Vector3D(1.0, 0.0, 0.0))
        p2.foo = 'bar'
        pdump = pickle.dumps((p1, p2))

        newp1, newp2 = pickle.loads(pdump)
        self.assertLess(
            IMP.algebra.get_distance(p1.get_point_on_plane(),
                                     newp1.get_point_on_plane()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(p1.get_normal(),
                                     newp1.get_normal()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(p2.get_point_on_plane(),
                                     newp2.get_point_on_plane()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(p2.get_normal(),
                                     newp2.get_normal()), 1e-4)
        self.assertEqual(newp2.foo, 'bar')


if __name__ == '__main__':
    IMP.test.main()
