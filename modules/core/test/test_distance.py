import IMP
import IMP.core
import IMP.test
import StringIO

class Tests(IMP.test.TestCase):
    """Test distance restraint and three harmonic score functions"""

    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)

        self.imp_model = IMP.Model()
        self.particles = []
        self.rsrs = []

        # create particles 0 - 1
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         -3.0, 0.0, 0.0))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         0.0, 0.0, 0.0))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         4.0, 0.0, 0.0))

        p1 = self.particles[0]
        self.radkey= IMP.FloatKey("radius")
        p1.add_attribute(self.radkey, 1.0, False)
        p1 = self.particles[1]
        p1.add_attribute(self.radkey, 2.0, False)
        p1 = self.particles[2]
        p1.add_attribute(self.radkey, 3.0, False)

    def _add_radius_restraints(self, p1, p2):
        """Add upper bound, lower bound, and harmonic distance restraints
           between p1 and p2 based on their radii."""
        mean = p1.get_value(self.radkey) + p2.get_value(self.radkey)
        for sf in (IMP.core.HarmonicUpperBound(mean, 0.1),
                   IMP.core.HarmonicLowerBound(mean, 0.1),
                   IMP.core.Harmonic(mean, 0.1)):
            r = IMP.core.DistanceRestraint(sf, p1, p2)
            p1.get_model().add_restraint(r)
            self.rsrs.append(r)

    def _make_restraints(self):
        """Build a set of test distance restraints on the particles"""

        # all should be 0.0
        self._add_radius_restraints(self.particles[1], self.particles[0])

        # exceed lower bound
        self._add_radius_restraints(self.particles[1], self.particles[2])

        # exceed upper bound
        self._add_radius_restraints(self.particles[0], self.particles[2])

        # all should be 0.0
        for fs in (IMP.core.HarmonicUpperBound(3.0, 0.1),
                   IMP.core.HarmonicLowerBound(3.0, 0.1),
                   IMP.core.Harmonic(3.0, 0.1)):
            r = IMP.core.DistanceRestraint(fs, self.particles[1],
                                           self.particles[0])
            self.particles[1].get_model().add_restraint(r)
            self.rsrs.append(r)

        # exceed lower bound
        for fs in (IMP.core.HarmonicUpperBound(5.0, 0.1),
                   IMP.core.HarmonicLowerBound(5.0, 0.1),
                   IMP.core.Harmonic(5.0, 0.1)):
            r = IMP.core.DistanceRestraint(fs, self.particles[1],
                                           self.particles[2])
            self.particles[1].get_model().add_restraint(r)
            self.rsrs.append(r)

        # exceed upper bound
        for fs in (IMP.core.HarmonicUpperBound(4.0, 0.1),
                   IMP.core.HarmonicLowerBound(4.0, 0.1),
                   IMP.core.Harmonic(4.0, 0.1)):
            r = IMP.core.DistanceRestraint(fs, self.particles[0],
                                           self.particles[2])
            self.particles[0].get_model().add_restraint(r)
            self.rsrs.append(r)

    def test_distance(self):
        """Test that distance restraints are reasonable"""
        self._make_restraints()

        # score should not change with deriv calcs
        accum = IMP.DerivativeAccumulator()

        # score should be equivalent if attribute is used or equivalent hard-coded distance is used
        for i in range(9):
            self.assertEqual(self.rsrs[i].evaluate(False),
                             self.rsrs[i+9].evaluate(False),
                             "should get same distance whether explicit "
                             "or through radii")

        # exact match
        self.assertEqual(self.rsrs[0].evaluate(False), 0.0, "unexpected distance score")
        self.assertEqual(self.rsrs[1].evaluate(False), 0.0, "unexpected distance score")
        self.assertEqual(self.rsrs[2].evaluate(False), 0.0, "unexpected distance score")

        # too close
        self.assertEqual(self.rsrs[0].evaluate(False), 0.0, "unexpected distance score")
        self.assertTrue(self.rsrs[1].evaluate(False) == self.rsrs[2].evaluate(False) == 0.0, "unexpected distance score")

        # too far
        self.assertEqual(self.rsrs[1].evaluate(False), 0.0, "unexpected distance score")
        self.assertTrue(self.rsrs[0].evaluate(False) == self.rsrs[2].evaluate(False) == 0.0, "unexpected distance score")

if __name__ == '__main__':
    IMP.test.main()
