import IMP
import IMP.test
import IMP.algebra
import math


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
        print e
        self.assertLess((e.get_radii() - radii).get_magnitude(), 1e-4)
        newr = e.get_reference_frame()
        self.assertLess((newr.get_transformation_to().get_translation()
                         - tran).get_magnitude(), 1e-5)
        self.assertLess((e.get_transformation().get_translation()
                         - tran).get_magnitude(), 1e-5)
        self.assertLess((e.get_center() - tran).get_magnitude(), 1e-5)
        for i in range(3):
            self.assertAlmostEqual(e.get_radius(i), radii[i], delta=1e-5)

    def test_construction(self):
        """Check Ellipsoid construction"""
        r = IMP.algebra.get_rotation_about_axis(
                 IMP.algebra.Vector3D(0., 0., 0.), .1)
        e = IMP.algebra.Ellipsoid3D(IMP.algebra.Vector3D(0.0, 1.0, 2.0),
                                    5.0, 8.0, 10.0, r)
        self.assertEqual([x for x in e.get_radii()], [5., 8., 10.])
        er = r / e.get_rotation()
        for c, exp_c in zip(er.get_quaternion(), [1., 0., 0., 0.]):
            self.assertAlmostEqual(c, exp_c, delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
