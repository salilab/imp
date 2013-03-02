import os
import IMP
import IMP.core
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    """Class to test simple distance restraint"""

    def load_particles(self, m, coordinates):
        """Load particles"""
        self.particles = []

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
        r = IMP.restrainer.create_simple_distance(self.particles).get_restraint()
        # Make sure that refcounting is working
        self.assertEqual(r.get_ref_count(), 1)
        self.imp_model.add_restraint(r)
        self.opt.optimize(1000)
        self.assertLess(r.evaluate(False), 0.1)


    def test_methods(self):
        """Check SimpleDistance's methods"""

        sd = IMP.restrainer.create_simple_distance(self.particles)
        r = sd.get_restraint()
        self.imp_model.add_restraint(r)
        h = sd.get_harmonic_upper_bound()
        # Make sure that refcounting is working
        # refs from Python, the SimpleDistance object, and the model
        self.assertEqual(r.get_ref_count(), 3)
        # refs from Python, the SimpleDistance object, and the Restraint
        self.assertEqual(h.get_ref_count(), 3)

        sd.set_mean(10.0)
        self.assertAlmostEqual(h.get_mean(), 10.0, delta=1e-4)

        sd.set_standard_deviation(3.5)
        self.assertAlmostEqual(h.get_k(),
                               h.get_k_from_standard_deviation(3.5), delta=1e-4)
        sd.set_k(0.1)
        self.assertAlmostEqual(h.get_k(), 0.1, delta=1e-4)

        r.set_was_used(True)
        r.show()

        self.imp_model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
