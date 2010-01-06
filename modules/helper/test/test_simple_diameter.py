import unittest
import os
import IMP
import IMP.core
import IMP.test
import IMP.helper

class SimpleDiameterTest(IMP.test.TestCase):
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
        sd = IMP.helper.create_simple_diameter(self.ps, self.diameter)
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
                d=IMP.core.distance(IMP.core.XYZ(p0.get_particle()),
                                    IMP.core.XYZ(p1.get_particle()))
                self.assert_(d < 1.1*self.diameter)

    def test_simple_diameter(self):
        """Test that simple diameter restraints are reasonable"""
        self.probabilistic_test("self._test_simple_diameter()", .002)


    def test_methods(self):
        """Check SimpleDiameter's methods"""

        sd = IMP.helper.create_simple_diameter(self.ps, self.diameter)
        r = sd.get_restraint()
        self.imp_model.add_restraint(r)
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
