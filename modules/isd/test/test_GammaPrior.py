#!/usr/bin/env python
import numpy
import random
import IMP
from IMP.isd import GammaPrior
from IMP.isd import Scale
import IMP.test


class Tests(IMP.test.TestCase):

    def initialize(self):
        m = IMP.Model()
        p = IMP.Particle(m)

        return m, p

    def test_evaluate_at(self):
        try:
            from scipy.stats import gamma
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        theta = 2.0
        kappa = 1.4

        m, p = self.initialize()

        gp = GammaPrior(m, p, kappa, theta)

        randnum = random.uniform(0.0,10.0)

        self.assertAlmostEqual(gp.evaluate_at(randnum), -1*numpy.log(gamma.pdf(randnum, kappa, 0.0, theta)))

    def test_unprotected_evaluate(self):
        try:
            from scipy.stats import gamma
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        theta = 2.0
        kappa = 1.6

        sigma_value = 15.0

        m,p = self.initialize()
        sigma = Scale.setup_particle(p, sigma_value)

        DA = IMP.DerivativeAccumulator()

        gp = GammaPrior(m, p, kappa, theta)

        self.assertAlmostEqual(gp.unprotected_evaluate(DA), -1*numpy.log(gamma.pdf(sigma_value/10, kappa, 0.0, theta)))


if __name__ == '__main__':
    IMP.test.main()
