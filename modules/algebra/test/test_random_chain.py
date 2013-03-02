import IMP.test
import IMP.algebra
import math
import itertools

class Tests(IMP.test.TestCase):
    def test_random_chain(self):
        """Testing the creation of a random chain"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        n=50
        r=1.2
        chain= IMP.algebra.get_random_chain(n,r)
        self.assertEqual(len(chain), n)
        for i in range(0, len(chain)-1):
            self.assertAlmostEqual(IMP.algebra.get_distance(chain[i], chain[i+1]),
                                   2*r, delta=.1*2*r)
            #print ".cylinder " + str(chain[i]
        for i in range(0, len(chain)):
            for j in range(0,i-1):
                self.assertGreater(IMP.algebra.get_distance(chain[i], chain[j]),
                                   .9*2*r)

    def test_random_chain_obst(self):
        """Testing the creation of a random chain with obstacles"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        n=50
        r=1.2
        v= IMP.algebra.Vector3D(1,2,3)
        ss= []
        for i in range(0,100):
            c= IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(10,10,10),
                                                 3.0))
            cr=r/2.0
            ss.append(IMP.algebra.Sphere3D(c,cr))
        chain= IMP.algebra.get_random_chain(n,r, v, ss)
        self.assertEqual(len(chain), n)
        self.assertAlmostEqual(v[0], chain[0][0], delta=.01)
        self.assertAlmostEqual(v[1], chain[0][1], delta=.01)
        self.assertAlmostEqual(v[2], chain[0][2], delta=.01)
        for i in range(0, len(chain)-1):
            self.assertAlmostEqual(IMP.algebra.get_distance(chain[i], chain[i+1]),
                                   2*r, delta=.1*2*r)
            #print ".cylinder " + str(chain[i]
        for i in range(0, len(chain)):
            for j in range(0,i-1):
                self.assertGreater(IMP.algebra.get_distance(chain[i], chain[j]),
                                   .9*2*r)
        for i in range(0, len(ss)):
            for c in chain:
                self.assertFalse(IMP.algebra.get_interiors_intersect(ss[i],IMP.algebra.Sphere3D(c, .9*r)))
if __name__ == '__main__':
    IMP.test.main()
