import unittest
import IMP.utils
import IMP.test, IMP
import random

class SteepestDescentTests(IMP.test.IMPTestCase):
    """Test steepest descent optimizer"""

    def setUp(self):
        """set up distance restraints and create optimizer """

        self.imp_model = IMP.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        # create particles 0 - 1
        self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                    -43.0, 65.0, 93.0))
        self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                    20.0, 74.0, -80.0))
        self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                    4.0, -39.0, 26.0))
        radkey= IMP.FloatKey("radius")

        p1 = self.particles[0]
        p1.add_attribute(radkey, 1.0, False)
        p1 = self.particles[1]
        p1.add_attribute(radkey, 2.0, False)
        p1 = self.particles[2]
        p1.add_attribute(radkey, 3.0, False)

        # separate 3 particles by their radii
        for pairs in ((0,1), (1,2), (0,2)):
            p1 = self.particles[pairs[0]]
            p2 = self.particles[pairs[1]]
            mean = p1.get_value(radkey) + p2.get_value(radkey)
            sf = IMP.Harmonic(mean, 0.1)
            rsr = IMP.DistanceRestraint(p1, p2, sf)
            self.rsrs.append(rsr)

        # add restraints
        rs = IMP.RestraintSet("distance_rsrs")
        self.imp_model.add_restraint(rs)
        self.restraint_sets.append(rs)
        for i in range(len(self.rsrs)):
            rs.add_restraint(self.rsrs[i])

        self.steepest_descent = IMP.SteepestDescent()
        self.steepest_descent.set_model(self.imp_model)



    def test_sd_optimizer1(self):
        """Test that optimizer brings particles together"""

        self.steepest_descent.optimize(50)

        for i in range(0, 2):
            for j in range(i+1, 3):
                dist = self.particle_distance(self.particles, i, j) \
                       - self.particles[i].get_value(IMP.FloatKey("radius")) \
                       - self.particles[j].get_value(IMP.FloatKey("radius"))
                self.assertAlmostEqual(0.0, dist, places=2)


    def test_sd_optimizer2(self):
        """Test that optimizer spreads particles apart"""

        # Start off with all particles in close proximity (but not actually
        # colocated, as the derivative of zero distance is zero):
        for p in self.particles:
            p.set_x(random.uniform(-0.01, 0.01))
            p.set_y(random.uniform(-0.01, 0.01))
            p.set_z(random.uniform(-0.01, 0.01))

        self.steepest_descent.optimize(50)

        for i in range(0, 2):
            for j in range(i+1, 3):
                dist = self.particle_distance(self.particles, i, j) \
                       - self.particles[i].get_value(IMP.FloatKey("radius")) \
                       - self.particles[j].get_value(IMP.FloatKey("radius"))
                self.assertAlmostEqual(0.0, dist, places=2)


if __name__ == '__main__':
    unittest.main()
