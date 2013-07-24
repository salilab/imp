import IMP
import IMP.test
import IMP.algebra
import math
import random

class Tests(IMP.test.TestCase):
    def test_conversion_from_ZYZ_to_quaternion(self):
        """Check Euler ZYZ conversions"""
        rot1 = 0.777
        tilt1 = 0.66
        psi1 = 0.0
        rot2 = 1.9
        tilt2 = 1.77
        psi2 = 0.0

        IMP.base.set_log_level(IMP.base.VERBOSE)
        r1 = IMP.algebra.get_rotation_from_fixed_zyz(rot1, tilt1, psi1)
        r2 = IMP.algebra.get_rotation_from_fixed_zyz(rot2, tilt2, psi2)
        d = IMP.algebra.get_distance(r1, r2)
        print "Out:"
        print d
        self.assertAlmostEqual(d, 0.39, delta=.1)


if __name__ == '__main__':
    IMP.test.main()
