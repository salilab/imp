import unittest
import IMP
import IMP.test
import IMP.utils
import IMP.core
import math

# Use faster built-in 'set' type on newer Pythons; fall back to the older
# 'sets' module on older Pythons
try:
    x = set
    del x
except NameError:
    import sets
    set = sets.Set

class AllPairsContainerTest(IMP.test.TestCase):
    """Tests for all pairs pair container"""

    def test_allp(self):
        """Check AllPairsPairContainer"""
        m= IMP.Model()
        for i in range(0,50):
            p= IMP.Particle(m)
        lp= IMP.core.ListSingletonContainer(m.get_particles())
        self.assertEqual(lp.get_ref_count(), 1)
        apc= IMP.core.AllPairsPairContainer(lp)
        self.assertEqual(lp.get_ref_count(), 2)
        # use that names are unique
        psl= set()
        print apc.get_number_of_particle_pairs()
        print lp.get_number_of_particles()
        IMP.set_log_level(IMP.VERBOSE)
        for i in range(0, apc.get_number_of_particle_pairs()):
            #print ap[-1][0]
            #print ap[-1][1]
            n0= apc.get_particle_pair(i)[0].get_name()
            n1= apc.get_particle_pair(i)[1].get_name()
            if n0 < n1: dp= (n0, n1)
            else: dp= (n1, n0)
            print dp
            #print psl
            self.assert_(not dp in psl, "Pair " + n0 + " and " +n1\
                             +" is already in list "+ str(psl))
            psl=psl.union([dp])
        print psl
        self.assertEqual(apc.get_number_of_particle_pairs(),
                         lp.get_number_of_particles()*(lp.get_number_of_particles()-1)/2)

if __name__ == '__main__':
    unittest.main()
