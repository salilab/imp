import unittest
import IMP.utils
import IMP.core
import IMP.test, IMP

class DistanceTests(IMP.test.TestCase):
    """Test distance restraint and three harmonic score functions"""

    def test_diagmeter(self):
        """Test that diameter restraints are reasonable"""

        diameter=10
        m= IMP.Model()
        lc= IMP.core.ListSingletonContainer(IMP.core.create_xyzr_particles(m, 50, 1.0))
        h=IMP.core.HarmonicUpperBound(0,1)
        r=IMP.core.DiameterRestraint(h, lc, diameter)
        m.add_restraint(r)

        # Set up optimizer
        o= IMP.core.ConjugateGradients()
        o.set_model(m)

        o.optimize(100)

        max=0
        for p0 in lc.get_particles():
            for p1 in lc.get_particles():
                d=IMP.core.distance(IMP.core.XYZDecorator(p0),
                                    IMP.core.XYZDecorator(p1))
                self.assert_(d < 1.1*diameter)


if __name__ == '__main__':
    unittest.main()
