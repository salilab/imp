import unittest
import IMP.test
import IMP.algebra
import math
import itertools

class Vector3DTests(IMP.test.TestCase):
    def test_random_chain(self):
        """Testing the creation of a random chain"""
        IMP.set_log_level(IMP.VERBOSE)
        n=50
        r=1.2
        chain= IMP.algebra.random_chain(n,r)
        self.assertEqual(len(chain), n);
        for i in range(0, len(chain)-1):
            self.assertInTolerance(IMP.algebra.distance(chain[i], chain[i+1]),
                                   2*r, .1*2*r)
            #print ".cylinder " + str(chain[i]
        for i in range(0, len(chain)):
            for j in range(0,i-1):
                self.assert_(IMP.algebra.distance(chain[i], chain[j]) > .9*2*r)

    def test_random_chain_obst(self):
        """Testing the creation of a random chain with obstacles"""
        IMP.set_log_level(IMP.VERBOSE)
        n=50
        r=1.2
        v= IMP.algebra.Vector3D(1,2,3)
        ss= IMP.algebra.Sphere3Ds()
        for i in range(0,100):
            c= IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(10,10,10),
                                                 3)
            cr=r/2.0
            ss.append(IMP.algebra.Sphere3D(c,cr))
        chain= IMP.algebra.random_chain(n,r, v, ss)
        self.assertEqual(len(chain), n)
        self.assertInTolerance(v[0], chain[0][0], .01)
        self.assertInTolerance(v[1], chain[0][1], .01)
        self.assertInTolerance(v[2], chain[0][2], .01)
        for i in range(0, len(chain)-1):
            self.assertInTolerance(IMP.algebra.distance(chain[i], chain[i+1]),
                                   2*r, .1*2*r)
            #print ".cylinder " + str(chain[i]
        for i in range(0, len(chain)):
            for j in range(0,i-1):
                self.assert_(IMP.algebra.distance(chain[i], chain[j]) > .9*2*r)
        for i in range(0, len(ss)):
            for c in chain:
                self.assert_(not IMP.algebra.interiors_intersect(ss[i],IMP.algebra.Sphere3D(c, .9*r)))
if __name__ == '__main__':
    unittest.main()
