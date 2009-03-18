import unittest
import os
import IMP
import IMP.test
import IMP.core
import IMP.utils

class ConnectivityTests(IMP.test.TestCase):
    """Class to test connectivity restraints"""

    def test_connectivity(self):
        """Test connectivity restraint.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins."""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()

        ps= self.create_particles_in_box(m,4)
        ds= map(lambda p: IMP.core.XYZDecorator.cast(p), ps)
        o = IMP.core.ConjugateGradients()
        o.set_threshold(1e-4)
        o.set_model(m)
        self.randomize_particles(m.get_particles(), 50.0)

        # add connectivity restraints

        ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
        ss= IMP.core.DistancePairScore(ub)
        r= IMP.core.ConnectivityRestraint(ss)
        m.add_restraint(r)
        r.add_particles(ps)
        o.optimize(1000)
        d01= IMP.core.distance(ds[0], ds[1])
        d02= IMP.core.distance(ds[0], ds[2])
        d03= IMP.core.distance(ds[0], ds[3])
        d12= IMP.core.distance(ds[1], ds[2])
        d13= IMP.core.distance(ds[1], ds[3])
        d23= IMP.core.distance(ds[2], ds[3])

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
        score= m.evaluate(False)
        self.assert_(sum >= 3,
                     "Wrong number of close pairs")
        self.assert_(ok01 or ok02 or ok03,
                     "Point 0 is not connected")
        self.assert_(ok01 or ok12 or ok13,
                     "Point 1 is not connected")
        self.assert_(ok02 or ok12 or ok23,
                     "Point 2 is not connected")
        self.assert_(ok03 or ok13 or ok23,
                     "Point 3 is not connected")
        self.assert_(score < 10, "Score too high")


if __name__ == '__main__':
    unittest.main()
