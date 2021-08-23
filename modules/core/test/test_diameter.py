import IMP
import IMP.core
import IMP.test
import IMP.container


class Tests(IMP.test.TestCase):

    """Test distance restraint and three harmonic score functions"""

    def _test_diameter(self):
        diameter = 10
        m = IMP.Model()
        lc = IMP.container.ListSingletonContainer(m,
                           IMP.core.create_xyzr_particles(m, 50, 1.0))
        h = IMP.core.HarmonicUpperBound(0, 1)
        r = IMP.core.DiameterRestraint(h, lc, diameter)
        sf = IMP.core.RestraintsScoringFunction([r])

        # Set up optimizer
        o = IMP.core.ConjugateGradients(m)
        o.set_scoring_function(sf)
        o.optimize(1000)

        max = 0
        for pi0 in lc.get_contents():
            for pi1 in lc.get_contents():
                d = IMP.core.get_distance(IMP.core.XYZR(m, pi0),
                                          IMP.core.XYZR(m, pi1))\
                    + 2 * (IMP.core.XYZR(m, pi0).get_radius()
                           + IMP.core.XYZR(m, pi1).get_radius())
                self.assertLess(d, 1.1 * diameter)

    def test_diameter(self):
        """Test that diameter restraints are reasonable"""
        self.probabilistic_check("self._test_diameter()", .002)

if __name__ == '__main__':
    IMP.test.main()
