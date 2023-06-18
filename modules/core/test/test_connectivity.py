from __future__ import print_function
import os
import IMP
import IMP.test
import IMP.core
import IMP.container


class Tests(IMP.test.TestCase):

    """Class to test connectivity restraints"""

    def test_connectivity(self):
        """Test connectivity restraint.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins."""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()

        ps = self.create_particles_in_box(m, 4)
        ds = [IMP.core.XYZ(p) for p in ps]
        o = IMP.core.ConjugateGradients(m)
        o.set_gradient_threshold(1e-4)
        self.randomize_particles(ps, 50.0)

        # add connectivity restraints

        ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
        ss = IMP.core.DistancePairScore(ub)
        lsc = IMP.container.ListSingletonContainer(m, ps)
        r = IMP.core.ConnectivityRestraint(ss, lsc)
        sf = IMP.core.RestraintsScoringFunction([r])
        o.set_scoring_function(sf)
        o.optimize(1000)
        d01 = IMP.core.get_distance(ds[0], ds[1])
        d02 = IMP.core.get_distance(ds[0], ds[2])
        d03 = IMP.core.get_distance(ds[0], ds[3])
        d12 = IMP.core.get_distance(ds[1], ds[2])
        d13 = IMP.core.get_distance(ds[1], ds[3])
        d23 = IMP.core.get_distance(ds[2], ds[3])

        ok01 = (d01 < 1.2)
        ok02 = (d02 < 1.2)
        ok12 = (d12 < 1.2)
        ok03 = (d03 < 1.2)
        ok13 = (d13 < 1.2)
        ok23 = (d23 < 1.2)
        print(ok01)
        print(ok02)
        print(ok12)
        print(ok03)
        print(ok13)
        print(ok23)
        sum = ok01 + ok02 + ok12 + ok03 + ok13 + ok23
        print(sum)
        print(d01)
        print(d02)
        print(d12)
        print(d03)
        print(d13)
        print(d23)
        score = sf.evaluate(False)
        self.assertGreaterEqual(sum, 3, "Wrong number of close pairs")
        self.assertTrue(ok01 or ok02 or ok03,
                        "Point 0 is not connected")
        self.assertTrue(ok01 or ok12 or ok13,
                        "Point 1 is not connected")
        self.assertTrue(ok02 or ok12 or ok23,
                        "Point 2 is not connected")
        self.assertTrue(ok03 or ok13 or ok23,
                        "Point 3 is not connected")
        self.assertLess(score, 10, "Score too high")
        pps = r.get_connected_index_pairs()
        lscore = 0
        for p in pps:
            lscore = lscore + ss.evaluate_index(m, (p[0], p[1]), None)
        self.assertAlmostEqual(score, lscore, delta=.1)

if __name__ == '__main__':
    IMP.test.main()
