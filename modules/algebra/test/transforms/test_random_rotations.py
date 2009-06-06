import unittest
import IMP
import IMP.test
import IMP.algebra
import random
import math

class RotationTests(IMP.test.TestCase):
    """Test rotations"""
    def test_uniform_random(self):
        rots= IMP.algebra.uniform_cover_rotations(100);
        pts= [IMP.algebra.Vector3D(1,0,0),
              IMP.algebra.Vector3D(0,1,0)]
        for r in rots:
            r0= r.rotate(pts[0])
            r1= r.rotate(pts[1])
            print ".color red"
            print ".sphere " + str(r0[0])+ " " + str(r0[1]) + " " + str(r0[2])\
                + " .1"
            print ".color blue"
            print ".sphere " + str(.5*r1[0])+ " " + str(.5*r1[1]) + " " \
                + str(.5*r1[2])\
                + " .1"

        c0= IMP.algebra.zeros()
        c1= c0
        for r in rots:
            c0= c0+ r.rotate(pts[0])
            c1= c1+ r.rotate(pts[1])
        c0/= len(rots)
        c1/= len(rots)
        for i in range(0,3):
            self.assertInTolerance(c0[i], 0, .1)
            self.assertInTolerance(c1[i], 0, .1)


if __name__ == '__main__':
    unittest.main()
