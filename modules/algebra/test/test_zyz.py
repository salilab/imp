import IMP
import IMP.test
import IMP.algebra
import math
import random

class Tests(IMP.test.TestCase):
    def test_cylinder_construction(self):
        """Check Euler ZYZ conversions"""
        rot= random.uniform(0,math.pi)
        tilt= random.uniform(0,math.pi)
        psi= random.uniform(0,math.pi)
        print "In"
        print rot
        print tilt
        print psi
        IMP.base.set_log_level(IMP.base.VERBOSE)
        r= IMP.algebra.get_rotation_from_fixed_zyz(rot, tilt, psi)
        e= IMP.algebra.get_fixed_zyz_from_rotation(r)
        print "Out"
        print e.get_rot()
        print e.get_tilt()
        print e.get_psi()
        self.assertAlmostEqual(rot, e.get_rot(), delta=.1)
        self.assertAlmostEqual(tilt, e.get_tilt(), delta=.1)
        self.assertAlmostEqual(psi, e.get_psi(), delta=.1)



if __name__ == '__main__':
    IMP.test.main()
