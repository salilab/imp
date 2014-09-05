import IMP
import IMP.test
import IMP.algebra
import math


class Tests(IMP.test.TestCase):
    """Test Ellipsoid3D"""

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
