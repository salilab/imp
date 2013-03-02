import IMP
import IMP.test
import IMP.algebra
import random
import math

class Tests(IMP.test.TestCase):
    """Test rotations"""
    def test_uniform_random(self):
        """Test uniform random rotation"""
        if not hasattr(IMP, 'cgal'):
            self.skipTest("IMP.cgal module disabled")
        print "Getting them"
        rots= IMP.algebra.get_uniform_cover_rotations_3d(100)
        print "got them"
        pts= [IMP.algebra.Vector3D(1,0,0),
              IMP.algebra.Vector3D(0,1,0)]
        print type(rots)
        print rots
        for r in rots:
            print r
            print type(r)
            print str(r)
            r0= r.get_rotated(pts[0])
            r1= r.get_rotated(pts[1])
            print ".color red"
            print ".sphere " + str(r0[0])+ " " + str(r0[1]) + " " + str(r0[2])\
                + " .1"
            print ".color blue"
            print ".sphere " + str(.5*r1[0])+ " " + str(.5*r1[1]) + " " \
                + str(.5*r1[2])\
                + " .1"

        c0= IMP.algebra.get_zero_vector_3d()
        c1= c0
        for r in rots:
            c0= c0+ r.get_rotated(pts[0])
            c1= c1+ r.get_rotated(pts[1])
        c0/= len(rots)
        c1/= len(rots)
        for i in range(0,3):
            self.assertAlmostEqual(c0[i], 0, delta=.3)
            self.assertAlmostEqual(c1[i], 0, delta=.3)


if __name__ == '__main__':
    IMP.test.main()
