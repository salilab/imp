import unittest
import IMP.test
import IMP.algebra
import math

def in_box(lb, ub, pt):
    for i in range(0,3):
        if pt[i] < lb[i]:
            return False
        elif pt[i] > ub[i]:
            return False
    return True

class Vector3DTests(IMP.test.TestCase):
    def test_magnitude(self):
        """Testing the distribution of vectors on a box"""
        lb= IMP.algebra.Vector3D(10,8, 12)
        ub= IMP.algebra.Vector3D(23, 25, 28)
        print ".color 1 0 0"
        print ".dotat "+str(lb[0])+" "+str(lb[1])+" "+str(lb[2])
        print ".dotat "+str(ub[0])+" "+str(ub[1])+" "+str(ub[2])
        print ".color .5 .5 .5"
        delta= IMP.algebra.Vector3D(.1, .1, .1)
        ilb= lb+delta
        iub= ub-delta
        olb= lb-delta
        oub= ub+delta
        for i in range(0, 100):
            rv= IMP.algebra.random_vector_on_box(lb, ub)
            print ".dotat "+str(rv[0])+" "+str(rv[1])+" "+str(rv[2])
            for i in range(0, 3):
                self.assert_(in_box(olb, oub, rv))
                self.assert_(not in_box(ilb, iub, rv))

if __name__ == '__main__':
    unittest.main()
