from __future__ import print_function
import IMP
import IMP.test
import IMP.algebra
import math
import pickle


class Tests(IMP.test.TestCase):
    """Test Ellipsoid3D"""

    def test_trivial(self):
        """Check trivial Ellipsoid construction"""
        e = IMP.algebra.Ellipsoid3D()

    def test_ref_frame(self):
        """Check construction of Ellipsoid from reference frame"""
        rot = IMP.algebra.get_identity_rotation_3d()
        tran = IMP.algebra.Vector3D(1, 2, 3)
        tf = IMP.algebra.Transformation3D(rot, tran)
        r = IMP.algebra.ReferenceFrame3D(tf)
        radii = IMP.algebra.Vector3D(10, 20, 30)
        e = IMP.algebra.Ellipsoid3D(r, radii)
        print(e)
        self.assertLess((e.get_radii() - radii).get_magnitude(), 1e-4)
        newr = e.get_reference_frame()
        self.assertLess((newr.get_transformation_to().get_translation()
                         - tran).get_magnitude(), 1e-5)
        self.assertLess((newr.get_transformation_to().get_translation()
                         - tran).get_magnitude(), 1e-5)

    def test_construction(self):
        """Check Ellipsoid construction"""
        r = IMP.algebra.get_rotation_about_axis(
                 IMP.algebra.Vector3D(0., 0., 0.), .1)
        e = IMP.algebra.Ellipsoid3D(IMP.algebra.Vector3D(0.0, 1.0, 2.0),
                                    5.0, 8.0, 10.0, r)
        self.assertEqual([x for x in e.get_radii()], [5., 8., 10.])
        rot = e.get_reference_frame().get_transformation_to().get_rotation()
        er = r / rot
        for c, exp_c in zip(er.get_quaternion(), [1., 0., 0., 0.]):
            self.assertAlmostEqual(c, exp_c, delta=1e-6)

    def test_pickle(self):
        """Test (un-)pickle of Ellipsoid3D"""
        r1 = IMP.algebra.get_rotation_about_axis(
                 IMP.algebra.Vector3D(0., 0., 0.), .1)
        e1 = IMP.algebra.Ellipsoid3D(IMP.algebra.Vector3D(0.0, 1.0, 2.0),
                                     5.0, 8.0, 10.0, r1)
        r2 = IMP.algebra.get_rotation_about_axis(
                 IMP.algebra.Vector3D(0., 1., 0.), .1)
        e2 = IMP.algebra.Ellipsoid3D(IMP.algebra.Vector3D(1.0, 2.0, 0.0),
                                     6.0, 9.0, 11.0, r2)
        e2.foo = 'bar'
        dump = pickle.dumps((e1, e2))

        newe1, newe2 = pickle.loads(dump)
        self._assert_equal(e1, newe1)
        self._assert_equal(e2, newe2)
        self.assertEqual(newe2.foo, 'bar')

    def _assert_equal(self, a, b):
        ta = a.get_reference_frame().get_transformation_to()
        tb = b.get_reference_frame().get_transformation_to()
        self.assertLess(
            IMP.algebra.get_distance(ta.get_rotation(),
                                     tb.get_rotation()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(ta.get_translation(),
                                     tb.get_translation()), 1e-4)
        self.assertLess(
            IMP.algebra.get_distance(a.get_radii(), b.get_radii()), 1e-4)


if __name__ == '__main__':
    IMP.test.main()
