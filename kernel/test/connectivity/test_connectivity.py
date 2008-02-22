import unittest
import os
import IMP
import IMP.test
import IMP.utils

class ConnectivityTests(IMP.test.TestCase):
    """Class to test connectivity restraints"""

    def min_distance(self, psa, psb):
        md=100
        for pa in psa:
            for pb in psb:
                da= IMP.XYZDecorator.cast(pa)
                db= IMP.XYZDecorator.cast(pb)
                d= IMP.distance(da,db)
                if (d < md):
                    md=d
        return md

    def test_connectivity(self):
        """Test connectivity restraint.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins."""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()

        rk= IMP.FloatKey("radius")

        p0=IMP.Particles()
        p1=IMP.Particles()
        p2=IMP.Particles()
        p3=IMP.Particles()
        for i in range(13):
            p= IMP.Particle()
            m.add_particle(p)
            d= IMP.XYZDecorator.create(p)
            d.set_coordinates_are_optimized(True)
            if (i %3 == 0):
                p0.append(p)
                p.add_attribute(rk, 1);
            elif (i %3 == 1):
                p1.append(p)
                p.add_attribute(rk, 2);
            else :
                p2.append(p)
                p.add_attribute(rk, 3);
        p= IMP.Particle()
        m.add_particle(p)
        d= IMP.XYZDecorator.create(p)
        d.set_coordinates_are_optimized(True)
        p3.append(p)
        p.add_attribute(rk, 1);

        o = IMP.ConjugateGradients()
        o.set_threshold(1e-4)
        o.set_model(m)
        self.randomize_particles(m.get_particles(), 50.0)

        # add connectivity restraints

        ub = IMP.HarmonicUpperBound(0.0, 0.1)
        ss= IMP.SphereDistancePairScore(ub)
        r= IMP.ConnectivityRestraint(ss)
        m.add_restraint(r)
        r.add_set(p0)
        r.add_set(p1)
        r.add_set(p2)
        r.add_set(p3)
        o.optimize(1000)
        d01= self.min_distance(p0, p1)
        d02= self.min_distance(p0, p2)
        d12= self.min_distance(p1, p2)
        d03= self.min_distance(p0, p3)
        d13= self.min_distance(p1, p3)
        d23= self.min_distance(p2, p3)
        ok01= (d01 < 3.5)
        ok02= (d02 < 4.5)
        ok12= (d12 < 5.5)
        ok03= (d03 < 2.5)
        ok13= (d13 < 3.5)
        ok23= (d23 < 4.5)
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
        self.assert_(score < 10, "Score too high");


if __name__ == '__main__':
    unittest.main()
