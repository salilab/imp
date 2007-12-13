import unittest
import IMP.utils
import IMP.test, IMP

class DistanceTests(IMP.test.IMPTestCase):
    """Test distance restraint and three harmonic score functions"""

    def setUp(self):
        """Set up model and particles"""

        self.imp_model = IMP.Model()
        self.particles = []
        self.rsrs = []

        # create particles 0 - 1
        self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                    -3.0, 0.0, 0.0))
        self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                    0.0, 0.0, 0.0))
        self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                    4.0, 0.0, 0.0))

        p1 = self.particles[0]
        self.radkey= IMP.FloatKey("radius")
        p1.add_attribute(self.radkey, 1.0, False)
        p1 = self.particles[1]
        p1.add_attribute(self.radkey, 2.0, False)
        p1 = self.particles[2]
        p1.add_attribute(self.radkey, 3.0, False)

    def _make_restraints(self):
        """Build a set of test distance restraints on the particles"""
        score_func_params_ub = IMP.BasicScoreFuncParams("harmonic_upper_bound", 0.0, 0.1)
        score_func_params_lb = IMP.BasicScoreFuncParams("harmonic_lower_bound", 0.0, 0.1)
        score_func_params_h = IMP.BasicScoreFuncParams("harmonic", 0.0, 0.1)

        # all should be 0.0
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[0], self.radkey, score_func_params_ub))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[0], self.radkey, score_func_params_lb))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[0], self.radkey, score_func_params_h))

        # exceed lower bound
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[2], self.radkey, score_func_params_ub))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[2], self.radkey, score_func_params_lb))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[2], self.radkey, score_func_params_h))

        # exceed upper bound
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[0], self.particles[2], self.radkey, score_func_params_ub))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[0], self.particles[2], self.radkey, score_func_params_lb))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[0], self.particles[2], self.radkey, score_func_params_h))

        # all should be 0.0
        score_func_params_ub = IMP.BasicScoreFuncParams("harmonic_upper_bound", 3.0, 0.1)
        score_func_params_lb = IMP.BasicScoreFuncParams("harmonic_lower_bound", 3.0, 0.1)
        score_func_params_h = IMP.BasicScoreFuncParams("harmonic", 3.0, 0.1)

        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[0], score_func_params_ub))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[0], score_func_params_lb))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[0], score_func_params_h))

        # exceed lower bound
        score_func_params_ub = IMP.BasicScoreFuncParams("harmonic_upper_bound", 5.0, 0.1)
        score_func_params_lb = IMP.BasicScoreFuncParams("harmonic_lower_bound", 5.0, 0.1)
        score_func_params_h = IMP.BasicScoreFuncParams("harmonic", 5.0, 0.1)

        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[2], score_func_params_ub))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[2], score_func_params_lb))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[1], self.particles[2], score_func_params_h))

        # exceed upper bound
        score_func_params_ub = IMP.BasicScoreFuncParams("harmonic_upper_bound", 4.0, 0.1)
        score_func_params_lb = IMP.BasicScoreFuncParams("harmonic_lower_bound", 4.0, 0.1)
        score_func_params_h = IMP.BasicScoreFuncParams("harmonic", 4.0, 0.1)

        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[0], self.particles[2], score_func_params_ub))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[0], self.particles[2], score_func_params_lb))
        self.rsrs.append(IMP.DistanceRestraint(self.imp_model, self.particles[0], self.particles[2], score_func_params_h))

    def test_show(self):
        """Test Restraint::show() method"""
        ub = IMP.BasicScoreFuncParams("harmonic_upper_bound", 0.0, 0.1)
        r = IMP.DistanceRestraint(self.imp_model, self.particles[1],
                                  self.particles[0], self.radkey, ub)
        r.show()

    def test_distance(self):
        """Test that distance restraints are reasonable"""
        self._make_restraints()

        # score should not change with deriv calcs
        accum = IMP.DerivativeAccumulator()
        for rsr in self.rsrs:
            self.assertAlmostEqual(rsr.evaluate(None), rsr.evaluate(accum),
                                   places=5)

        # score should be equivalent if attribute is used or equivalent hard-coded distance is used
        for i in range(9):
            self.assert_(self.rsrs[i].evaluate(None) == self.rsrs[i+9].evaluate(None), "should get same distance whether explicit or through radii")

        # exact match
        self.assert_(self.rsrs[0].evaluate(None) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[1].evaluate(None) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[2].evaluate(None) == 0.0, "unexpected distance score")

        # too close
        self.assert_(self.rsrs[0].evaluate(None) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[1].evaluate(None) == self.rsrs[2].evaluate(None) == 0.0, "unexpected distance score")

        # too far
        self.assert_(self.rsrs[1].evaluate(None) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[0].evaluate(None) == self.rsrs[2].evaluate(None) == 0.0, "unexpected distance score")

if __name__ == '__main__':
    unittest.main()
