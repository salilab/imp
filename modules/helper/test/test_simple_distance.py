import unittest
import os
import IMP
import IMP.core
import IMP.test
import IMP.helper

class SimpleDistanceTest(IMP.test.TestCase):
    """Class to test simple distance restraint"""

    def load_particles(self, m, coordinates):
        """Load particles"""
        self.particles = IMP.Particles()

        for pt in coordinates:
            p = self.create_point_particle(m, *pt)
            p.add_attribute(IMP.FloatKey("radius"), 1.0, False)
            self.particles.append(p)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.imp_model = IMP.Model()
        self.load_particles(self.imp_model, [(0, 0, 0),(100, 100, 100)])

        self.opt = IMP.core.ConjugateGradients()
        self.opt.set_threshold(1e-4)
        self.opt.set_model(self.imp_model)


    def test_simple_distance(self):
        """Test simple distance restraint"""
        r = IMP.helper.create_simple_distance(self.particles).get_restraint()
        self.opt.optimize(1000)
        self.assert_(r.evaluate(False) == 0.0, "unexpected distance score")


    def test_methods(self):
        """Check SimpleDistance's methods"""

        sd = IMP.helper.create_simple_distance(self.particles)
        r = sd.get_restraint()
        h = sd.get_harmonic_upper_bound()

        sd.set_mean(10.0)
        self.assertInTolerance (h.get_mean(), 10.0, 1e-4)

        sd.set_stddev(3.5)
        self.assertInTolerance (h.get_k(),
                                h.k_from_standard_deviation(3.5), 1e-4)
        sd.set_k(0.1)
        self.assertInTolerance (h.get_k(), 0.1, 1e-4)

        r.set_was_owned(True)
        r.show()

        self.imp_model.evaluate(False)

if __name__ == '__main__':
    unittest.main()
