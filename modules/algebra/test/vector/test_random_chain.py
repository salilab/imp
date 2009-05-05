import unittest
import IMP.test
import IMP.algebra
import math
import itertools

class Vector3DTests(IMP.test.TestCase):
    def test_random_chain(self):
        """Testing the creation of a random chain"""
        n=50
        r=1.2
        chain= IMP.algebra.random_chain(n,r)
        self.assertEqual(len(chain), n);
        for i in range(0, len(chain)-1):
            self.assertInTolerance(IMP.algebra.distance(chain[i], chain[i+1]),
                                   2*r, .1*2*r)
        for i in range(0, len(chain)):
            for j in range(0,i-1):
                self.assert_(IMP.algebra.distance(chain[i], chain[j]) > .9*2*r)

if __name__ == '__main__':
    unittest.main()
