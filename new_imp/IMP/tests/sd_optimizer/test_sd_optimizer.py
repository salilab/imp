import unittest

# set the appropriate search path
import sys
sys.path.append("../python_libs/")
import IMP_Utils
import IMP_Test, imp2

# Class to test steepest descent optimizer in IMP
class test_sd_optimizer(IMP_Test.IMPTestCase):
    """test steepest descent optimizer in IMP """

    def setUp(self):
        """set up distance restraints and create optimizer """

        self.imp_model = imp2.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        self.score_func_h = imp2.Harmonic()

        # create particles 0 - 1
        self.particles.append(IMP_Utils.Particle(self.imp_model, -43.0, 65.0, 93.0))
        self.particles.append(IMP_Utils.Particle(self.imp_model, 20.0, 74.0, -80.0))
        self.particles.append(IMP_Utils.Particle(self.imp_model, 4.0, -39.0, 26.0))

        p1 = self.particles[0].imp()
        p1.add_float("radius", 1.0, False)
        p1 = self.particles[1].imp()
        p1.add_float("radius", 2.0, False)
        p1 = self.particles[2].imp()
        p1.add_float("radius", 3.0, False)

        # separate 3 particles by their radii
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[1].imp(), "radius", 0.1, self.score_func_h))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[1].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_h))
        self.rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[0].imp(), self.particles[2].imp(), "radius", 0.1, self.score_func_h))

        # add restraints
        rs = imp2.Restraint_Set("distance_rsrs")
        self.imp_model.add_restraint_set(rs)
        self.restraint_sets.append(rs)
        for i in range(len(self.rsrs)):
            rs.add_restraint(self.rsrs[i])

        self.steepest_descent = imp2.Steepest_Descent()




    def test_sd_optimizer1(self):
        """ test that optimizer brings particles together """

        self.steepest_descent.optimize(self.imp_model, 50)

        for i in range(0, 2):
            for j in range(i+1, 3):
                dist = self.IMP_Distance(self.particles, i, j) - self.particles[i].get_float("radius") - self.particles[j].get_float("radius")
                self.assertAlmostEqual(0.0, dist, places=2)


    def test_sd_optimizer2(self):
        """ test that optimizer spreads particles apart """

        self.particles[0].set_x(0.0)
        self.particles[0].set_y(0.0)
        self.particles[0].set_z(0.0)

        self.particles[1].set_x(0.0)
        self.particles[1].set_y(0.0)
        self.particles[1].set_z(0.0)

        self.particles[2].set_x(0.0)
        self.particles[2].set_y(0.0)
        self.particles[2].set_z(0.0)

        self.steepest_descent.optimize(self.imp_model, 50)

        for i in range(0, 2):
            for j in range(i+1, 3):
                dist = self.IMP_Distance(self.particles, i, j) - self.particles[i].get_float("radius") - self.particles[j].get_float("radius")
                self.assertAlmostEqual(0.0, dist, places=2)


if __name__ == '__main__':
    unittest.main()
