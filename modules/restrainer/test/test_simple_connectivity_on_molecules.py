import os
import IMP
import IMP.test
import IMP.core
import IMP.restrainer

class Tests(IMP.test.TestCase):
    """Class to test simple connectivity restraint on molecules"""

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()

        self.ds = IMP.core.create_xyzr_particles(self.m, 4, 1.0)
        self.ps = self.m.get_particles()
        self.mhs = IMP.atom.Hierarchies()
        for p in self.ps:
            mh = IMP.atom.Hierarchy.setup_particle(p)
            self.mhs.append(mh)

        self.o = IMP.core.ConjugateGradients()
        self.o.set_threshold(1e-4)
        self.o.set_model(self.m)
        self.randomize_particles(self.ps, 50.0)

    def test_connectivity(self):
        """Test simple connectivity restraint on molecules.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins."""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        # add connectivity restraints

        sc= IMP.restrainer.create_simple_connectivity_on_molecules(self.mhs)
        r = sc.get_restraint()
        self.m.add_restraint(r)
        sdps= sc.get_sphere_distance_pair_score()
        # Make sure that refcounting is working correctly
        # refs from Python, the SimpleConnectivity object, and the Model
        self.assertEqual(r.get_ref_count(), 3)
        # refs from Python, the SimpleConnectivity object, and the Restraint
        self.assertEqual(sdps.get_ref_count(), 3)

        self.o.optimize(1000)

        d01= IMP.core.get_distance(self.ds[0], self.ds[1])
        d02= IMP.core.get_distance(self.ds[0], self.ds[2])
        d03= IMP.core.get_distance(self.ds[0], self.ds[3])
        d12= IMP.core.get_distance(self.ds[1], self.ds[2])
        d13= IMP.core.get_distance(self.ds[1], self.ds[3])
        d23= IMP.core.get_distance(self.ds[2], self.ds[3])

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
        self.assertGreaterEqual(sum, 3, "Wrong number of close pairs")
        self.assertTrue(ok01 or ok02 or ok03, "Point 0 is not connected")
        self.assertTrue(ok01 or ok12 or ok13, "Point 1 is not connected")
        self.assertTrue(ok02 or ok12 or ok23, "Point 2 is not connected")
        self.assertTrue(ok03 or ok13 or ok23, "Point 3 is not connected")
        self.assertLess(score, 10, "Score too high")
        pps= r.get_connected_pairs()
        lscore=0
        for p in pps:
            lscore= lscore+sdps.evaluate((p[0], p[1]), None)
        self.assertAlmostEqual(score, lscore, delta=.1)

    def test_methods(self):
        """Check SimpleConnectivity's methods for molecules"""

        sc = IMP.restrainer.create_simple_connectivity_on_molecules(self.mhs)
        r = sc.get_restraint()
        h = sc.get_harmonic_upper_bound()
        sdps = sc.get_sphere_distance_pair_score()

        sc.set_mean(10.0)
        self.assertAlmostEqual(h.get_mean(), 10.0, delta=1e-4)

        sc.set_standard_deviation(3.5)
        self.assertAlmostEqual(h.get_k(),
                               h.get_k_from_standard_deviation(3.5), delta=1e-4)
        sc.set_k(0.1)
        self.assertAlmostEqual(h.get_k(), 0.1, delta=1e-4)

        r.set_was_used(True)
        r.show()

        self.m.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
