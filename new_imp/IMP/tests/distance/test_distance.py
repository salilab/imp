import unittest

# set the appropriate search path
import sys
sys.path.append("../python_libs/")
import IMP_Utils
import IMP_Test, imp2

# Class to test particles
class test_distance(IMP_Test.IMPTestCase):
    """test distance restraint and three harmonic score functions """

    def setUp(self):
        """set up distance restraints """

        self.imp_model = imp2.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        self.score_func_ub = imp2.Harmonic_Upper_Bound()
        self.score_func_lb = imp2.Harmonic_Lower_Bound()
        self.score_func_h = imp2.Harmonic()

        # create particles 0 - 1
        self.particles.append(IMP_Utils.Particle(self.imp_model, -3.0, 0.0, 0.0))
        self.particles.append(IMP_Utils.Particle(self.imp_model, 0.0, 0.0, 0.0))
        self.particles.append(IMP_Utils.Particle(self.imp_model, 4.0, 0.0, 0.0))

        p1 = self.particles[0].imp()
        p1.add_float("radius", 1.0, False)
        p1 = self.particles[1].imp()
        p1.add_float("radius", 2.0, False)
        p1 = self.particles[2].imp()
        p1.add_float("radius", 3.0, False)

        # all should be 0.0
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[0].imp(), "radius", 0.1, self.score_func_ub))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[0].imp(), "radius", 0.1, self.score_func_lb))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[0].imp(), "radius", 0.1, self.score_func_h))

        # exceed lower bound
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_ub))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_lb))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_h))

        # exceed upper bound
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_ub))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_lb))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_h))

        # all should be 0.0
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[0].imp(), 3.0, 0.1, self.score_func_ub))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[0].imp(), 3.0, 0.1, self.score_func_lb))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[0].imp(), 3.0, 0.1, self.score_func_h))

        # exceed lower bound
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), 5.0, 0.1, self.score_func_ub))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), 5.0, 0.1, self.score_func_lb))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), 5.0, 0.1, self.score_func_h))

        # exceed upper bound
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), 4.0, 0.1, self.score_func_ub))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), 4.0, 0.1, self.score_func_lb))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), 4.0, 0.1, self.score_func_h))

        # add restraints
        rs = imp2.Restraint_Set("distance_rsrs")
        self.restraint_sets.append(rs)
        for i in range(len(self.rsrs)):
            rs.add_restraint(self.rsrs[i])


    def test_distance(self):
        """ test that distance restraints are reasonable """

        # score should not change with deriv calcs
        for i in range(len(self.rsrs)):
            self.assert_(self.rsrs[i].evaluate(False) == self.rsrs[i].evaluate(True), "score should be independent of whether derivatives are calcuated or not")

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
