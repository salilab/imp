import os
import IMP
import IMP.core
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    """Class to test simple diameter restraint"""

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)

        self.imp_model = IMP.Model()
        self.ps = IMP.core.create_xyzr_particles(self.imp_model, 50, 1.0)
        self.diameter = 10

        self.opt = IMP.core.ConjugateGradients()
        self.opt.set_threshold(1e-4)
        self.opt.set_model(self.imp_model)


    def _test_simple_diameter(self):
        """Test simple diameter restraint"""
        sd = IMP.restrainer.create_simple_diameter(self.ps, self.diameter)
        r = sd.get_restraint()
        self.imp_model.add_restraint(r)
        # Make sure that refcounting is working
        # refs from Python, the SimpleDiameter object, and the model
        self.assertEqual(r.get_ref_count(), 3)
        # refs from Python, the SimpleDiameter object, and the Restraint
        h = sd.get_harmonic_upper_bound()
        self.assertEqual(h.get_ref_count(), 3)

        self.opt.optimize(1000)

        for p0 in self.ps:
            for p1 in self.ps:
                d=IMP.core.get_distance(IMP.core.XYZ(p0.get_particle()),
                                    IMP.core.XYZ(p1.get_particle()))
                self.assertLess(d, 1.1*self.diameter)

    def test_simple_diameter(self):
        """Test that simple diameter restraints are reasonable"""
        self.probabilistic_test("self._test_simple_diameter()", .002)


    def test_methods(self):
        """Check SimpleDiameter's methods"""

        sd = IMP.restrainer.create_simple_diameter(self.ps, self.diameter)
        r = sd.get_restraint()
        self.imp_model.add_restraint(r)
        h = sd.get_harmonic_upper_bound()

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
