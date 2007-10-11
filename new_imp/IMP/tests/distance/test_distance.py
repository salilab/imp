import unittest
import IMP.utils
import IMP.test, IMP

# Class to test particles
class test_distance(IMP.test.IMPTestCase):
    """test distance restraint and three harmonic score functions """

    def setUp(self):
        """set up distance restraints """

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
        p1.add_float("radius", 1.0, False)
        p1 = self.particles[1]
        p1.add_float("radius", 2.0, False)
        p1 = self.particles[2]
        p1.add_float("radius", 3.0, False)

        score_func_params_ub = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 0.0, 0.1)
        score_func_params_lb = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 0.0, 0.1)
        score_func_params_h = IMP.Basic_Score_Func_Params("harmonic", 0.0, 0.1)

        # all should be 0.0
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[0], "radius", score_func_params_ub))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[0], "radius", score_func_params_lb))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[0], "radius", score_func_params_h))

        # exceed lower bound
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[2], "radius", score_func_params_ub))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[2], "radius", score_func_params_lb))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[2], "radius", score_func_params_h))

        # exceed upper bound
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[0], self.particles[2], "radius", score_func_params_ub))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[0], self.particles[2], "radius", score_func_params_lb))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[0], self.particles[2], "radius", score_func_params_h))

        # all should be 0.0
        score_func_params_ub = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 3.0, 0.1)
        score_func_params_lb = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 3.0, 0.1)
        score_func_params_h = IMP.Basic_Score_Func_Params("harmonic", 3.0, 0.1)

        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[0], score_func_params_ub))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[0], score_func_params_lb))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[0], score_func_params_h))

        # exceed lower bound
        score_func_params_ub = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 5.0, 0.1)
        score_func_params_lb = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 5.0, 0.1)
        score_func_params_h = IMP.Basic_Score_Func_Params("harmonic", 5.0, 0.1)

        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[2], score_func_params_ub))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[2], score_func_params_lb))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[1], self.particles[2], score_func_params_h))

        # exceed upper bound
        score_func_params_ub = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 4.0, 0.1)
        score_func_params_lb = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 4.0, 0.1)
        score_func_params_h = IMP.Basic_Score_Func_Params("harmonic", 4.0, 0.1)

        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[0], self.particles[2], score_func_params_ub))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[0], self.particles[2], score_func_params_lb))
        self.rsrs.append(IMP.RSR_Distance(self.imp_model, self.particles[0], self.particles[2], score_func_params_h))


    def test_distance(self):
        """ test that distance restraints are reasonable """

        # score should not change with deriv calcs
        for rsr in self.rsrs:
            self.assertAlmostEqual(rsr.evaluate(False), rsr.evaluate(True),
                                   places=5)

        # score should be equivalent if attribute is used or equivalent hard-coded distance is used
        for i in range(9):
            self.assert_(self.rsrs[i].evaluate(False) == self.rsrs[i+9].evaluate(False), "should get same distance whether explicit or through radii")

        # exact match
        self.assert_(self.rsrs[0].evaluate(False) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[1].evaluate(False) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[2].evaluate(False) == 0.0, "unexpected distance score")

        # too close
        self.assert_(self.rsrs[0].evaluate(False) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[1].evaluate(False) == self.rsrs[2].evaluate(False) == 0.0, "unexpected distance score")

        # too far
        self.assert_(self.rsrs[1].evaluate(False) == 0.0, "unexpected distance score")
        self.assert_(self.rsrs[0].evaluate(False) == self.rsrs[2].evaluate(False) == 0.0, "unexpected distance score")

if __name__ == '__main__':
    unittest.main()
