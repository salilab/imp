import unittest
import IMP.utils
import IMP.test, IMP

class RestraintSetTests(IMP.test.IMPTestCase):
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
        score_func_params = IMP.BasicScoreFuncParams("harmonic", 5.0, 0.1)

        self.distrsr = IMP.DistanceRestraint(self.model, self.particles[0],
                                             self.particles[1],
                                             score_func_params)

        # add restraints
        self.rset = IMP.RestraintSet("distance_rsrs")
        self.model.add_restraint(self.rset)
        self.rset.add_restraint(self.distrsr)

    def test_weights(self):
        """Test that sets can be weighted"""
        model_data = self.model.get_model_data()
        fidx = self.particles[0].get_float_index("x")
        e1 = self.model.evaluate(True)
        d1 = model_data.get_deriv(fidx)
        self.rset.set_weight(0.5)
        e2 = self.model.evaluate(True)
        d2 = model_data.get_deriv(fidx)
        self.assertAlmostEqual(e1 * 0.5, e2, places=3)
        self.assertAlmostEqual(d1 * 0.5, d2, places=3)

if __name__ == '__main__':
    unittest.main()
