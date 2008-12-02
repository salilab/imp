import unittest
import IMP
import IMP.test
import IMP.utils
import IMP.core
import math

class AllPairsContainerTest(IMP.test.TestCase):
    """Tests for all pairs pair container"""

    def test_allp(self):
        """Check AllPairsPairContainer"""
        m= IMP.Model()
        for i in range(0,20):
            p= IMP.Particle()
            m.add_particle(p)
        lp= IMP.core.ListSingletonContainer(m.get_particles())
        apc= IMP.core.AllPairsPairContainer(lp)
        self.assert_(not lp.thisown)
        ap=[]
        for i in range(0, apc.get_number_of_particle_pairs()):
            ap.append(apc.get_particle_pair(i))
            print ap[-1][0]
            print ap[-1][1]
        self.assertEqual(len(ap), 20*19/2)

if __name__ == '__main__':
    unittest.main()
