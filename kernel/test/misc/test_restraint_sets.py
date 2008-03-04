import unittest
import IMP.utils
import IMP.test, IMP

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def setUp(self):
        """Set up distance restraints"""
        self.model = IMP.Model()
        self.particles = []

        # create particles
        self.particles.append(IMP.utils.XYZParticle(self.model,
                                                    -43.0, 65.0, 93.0))
        self.particles.append(IMP.utils.XYZParticle(self.model,
                                                    20.0, 74.0, -80.0))

        # separate particles by 5.0:
        self.distrsr = IMP.DistanceRestraint(IMP.Harmonic(5.0, 0.1),
                                             self.particles[0],
                                             self.particles[1])

        # add restraints
        self.rset = IMP.RestraintSet("distance_rsrs")
        self.model.add_restraint(self.rset)
        self.rset.add_restraint(self.distrsr)

    def test_weights(self):
        """Test that sets can be weighted"""
        e1 = self.model.evaluate(True)
        d1 = self.particles[0].get_derivative(IMP.FloatKey("x"))
        self.rset.set_weight(0.5)
        e2 = self.model.evaluate(True)
        d2 = self.particles[0].get_derivative(IMP.FloatKey("x"))
        self.assertAlmostEqual(e1 * 0.5, e2, places=3)
        self.assertAlmostEqual(d1 * 0.5, d2, places=3)

if __name__ == '__main__':
    unittest.main()
