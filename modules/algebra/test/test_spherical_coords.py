import IMP
import IMP.test
import IMP.algebra
from math import pi
import pickle


class Tests(IMP.test.TestCase):

    def test_spherical_coords_conversions(self):
        """Check that spherical coords work"""
        self.assertRaises(ValueError, IMP.algebra.SphericalVector3D,
                          -1., pi / 2, 0.0)
        self.assertRaises(ValueError, IMP.algebra.SphericalVector3D,
                          1., -2. * pi, 0.0)
        self.assertRaises(ValueError, IMP.algebra.SphericalVector3D,
                          1., 0.0, 3. * pi)
        r = IMP.algebra.SphericalVector3D(1., pi / 2, 0.0)
        p = r.get_cartesian_coordinates()
        self.assertAlmostEqual(p[0], 1.0, places=3)
        self.assertAlmostEqual(p[1], 0.0, places=3)
        self.assertAlmostEqual(p[2], 0.0, places=3)
        x = IMP.algebra.Vector3D(1., 0., 0.)
        r = IMP.algebra.SphericalVector3D(x)
        self.assertAlmostEqual(r[0], 1.0, places=3)
        self.assertAlmostEqual(r[1], pi / 2., places=3)
        self.assertAlmostEqual(r[2], 0., places=3)

    def test_pickle(self):
        """Test (un-)pickle of SphericalVector3D"""
        r1 = IMP.algebra.SphericalVector3D(1., pi / 2, 0.0)
        r2 = IMP.algebra.SphericalVector3D(IMP.algebra.Vector3D(1., 0., 0.))
        r2.foo = 'bar'
        rdump = pickle.dumps((r1, r2))

        newr1, newr2 = pickle.loads(rdump)
        self.assertAlmostEqual(r1[0], newr1[0], delta=1e-4)
        self.assertAlmostEqual(r1[1], newr1[1], delta=1e-4)
        self.assertAlmostEqual(r1[2], newr1[2], delta=1e-4)
        self.assertAlmostEqual(r2[0], newr2[0], delta=1e-4)
        self.assertAlmostEqual(r2[1], newr2[1], delta=1e-4)
        self.assertAlmostEqual(r2[2], newr2[2], delta=1e-4)
        self.assertEqual(newr2.foo, 'bar')


if __name__ == '__main__':
    IMP.test.main()
