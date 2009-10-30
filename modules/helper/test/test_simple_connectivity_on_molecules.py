import unittest
import os
import IMP
import IMP.test
import IMP.core
import IMP.helper

class SimpleConnectivityTests(IMP.test.TestCase):
    """Class to test simple connectivity restraint on molecules"""

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()

        self.ds = IMP.core.create_xyzr_particles(self.m, 4, 1.0)
        self.ps = self.m.get_particles()

        self.o = IMP.core.ConjugateGradients()
        self.o.set_threshold(1e-4)
        self.o.set_model(self.m)
        self.randomize_particles(self.ps, 50.0)

    def test_connectivity(self):
        """Test simple connectivity restraint on molecules.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins."""
        IMP.set_log_level(IMP.VERBOSE)
        # add connectivity restraints

        sc= IMP.helper.create_simple_connectivity_on_molecules(self.ps)
        r = sc.restraint()
        sdps= sc.sphere_distance_pair_score()

        self.o.optimize(1000)

        d01= IMP.core.distance(self.ds[0], self.ds[1])
        d02= IMP.core.distance(self.ds[0], self.ds[2])
        d03= IMP.core.distance(self.ds[0], self.ds[3])
        d12= IMP.core.distance(self.ds[1], self.ds[2])
        d13= IMP.core.distance(self.ds[1], self.ds[3])
        d23= IMP.core.distance(self.ds[2], self.ds[3])

        ok01= (d01 < 1.2)
        ok02= (d02 < 1.2)
        ok12= (d12 < 1.2)
        ok03= (d03 < 1.2)
        ok13= (d13 < 1.2)
        ok23= (d23 < 1.2)
        print ok01
        print ok02
        print ok12
        print ok03
        print ok13
        print ok23
        sum = ok01 + ok02 + ok12 + ok03 + ok13 + ok23
        print sum
        print d01
        print d02
        print d12
        print d03
        print d13
        print d23
        score= self.m.evaluate(False)
        self.assert_(sum >= 3, "Wrong number of close pairs")
        self.assert_(ok01 or ok02 or ok03, "Point 0 is not connected")
        self.assert_(ok01 or ok12 or ok13, "Point 1 is not connected")
        self.assert_(ok02 or ok12 or ok23, "Point 2 is not connected")
        self.assert_(ok03 or ok13 or ok23, "Point 3 is not connected")
        self.assert_(score < 10, "Score too high")
        pps= r.get_connected_pairs()
        lscore=0
        for p in pps:
            lscore= lscore+sdps.evaluate(p[0], p[1], None)
        self.assertInTolerance(score, lscore, .1)

    def test_methods(self):
        """Check SimpleConnectivity's methods for molecules"""

        sc = IMP.helper.create_simple_connectivity_on_molecules(self.ps)
        r = sc.restraint()
        h = sc.harmonic_upper_bound()
        sdps = sc.sphere_distance_pair_score()

        sc.set_mean(10.0)
        self.assertInTolerance (h.get_mean(), 10.0, 1e-4)

        sc.set_stddev(3.5)
        self.assertInTolerance (h.get_k(),
                                h.k_from_standard_deviation(3.5), 1e-4)
        sc.set_k(0.1)
        self.assertInTolerance (h.get_k(), 0.1, 1e-4)

        r.set_was_owned(True)
        r.show()

        self.m.evaluate(False)

if __name__ == '__main__':
    unittest.main()
