import unittest
import IMP
import IMP.test
import IMP.algebra
import math
import random

class CylinderTests(IMP.test.TestCase):
    def test_cylinder_construction(self):
        """Check Euler ZYZ conversions"""
        rot= random.uniform(0,math.pi)
        tilt= random.uniform(0,math.pi)
        psi= random.uniform(0,math.pi)
        print "In"
        print rot
        print tilt
        print psi
        IMP.set_log_level(IMP.VERBOSE)
        r= IMP.algebra.rotation_from_fixed_zyz(rot, tilt, psi)
        e= IMP.algebra.fixed_zyz_from_rotation(r)
        print "Out"
        print e.get_rot()
        print e.get_tilt()
        print e.get_psi()
        self.assertInTolerance(rot, e.get_rot(), .1)
        self.assertInTolerance(tilt, e.get_tilt(), .1)
        self.assertInTolerance(psi, e.get_psi(), .1)



if __name__ == '__main__':
    unittest.main()
