import math
import random
import numpy as np

import IMP
import IMP.core
import IMP.test


def _get_beta(N, b):
    return 3. / (2. * N * b**2)


def _get_score(z, N, b):
    beta = _get_beta(N, b)
    return beta * z**2 - math.log(2 * beta * z)


def _get_derv(z, N, b):
    beta = _get_beta(N, b)
    return 2 * beta * z - 1. / float(z)


def _get_linear_score(z, N, b):
    slope = _get_linear_derv(N, b)
    intercept = 5.258546595708 - .5 * math.log(_get_beta(N, b))
    return slope * z + intercept


def _get_linear_derv(N, b):
    return -141.407214101686 * _get_beta(N, b)**.5


class Tests(IMP.test.TestCase):

    """Tests for SurfaceTetheredChain."""

    def test_init(self):
        """Test correct initialization."""
        func = IMP.core.SurfaceTetheredChain(10, 8)
        func.set_was_used(True)

    def test_evaluate(self):
        """Test evaluates to correct scores and derivatives."""
        for i in range(100):
            N = random.randint(1, 10)
            b = random.uniform(.1, 5.)
            func = IMP.core.SurfaceTetheredChain(N, b)
            func.set_was_used(True)
            max_z = 2 * N * b
            beta = _get_beta(N, b)
            min_z = .01 / (2 * beta)**.5
            z_range = np.linspace(min_z, max_z, 100)
            for z in z_range:
                corr_score = _get_score(z, N, b)
                corr_derv = _get_derv(z, N, b)
                score, deriv = func.evaluate_with_derivative(z)
                scoreonly = func.evaluate(z)
                self.assertAlmostEqual(scoreonly, corr_score, delta=1e-4)
                self.assertAlmostEqual(score, corr_score, delta=1e-4)
                self.assertAlmostEqual(deriv, corr_derv, delta=1e-4)

    def test_evaluate_linear(self):
        """Test linear region evaluates to correct scores and derivatives."""
        for i in range(100):
            N = random.randint(3, 10)
            b = random.uniform(.1, 5.)
            func = IMP.core.SurfaceTetheredChain(N, b)
            func.set_was_used(True)
            beta = _get_beta(N, b)
            min_z = .01 / (2 * beta)**.5
            z_range = np.linspace(-1, min_z, 100)
            corr_derv = _get_linear_derv(N, b)
            for z in z_range:
                corr_score = _get_linear_score(z, N, b)
                score, deriv = func.evaluate_with_derivative(z)
                scoreonly = func.evaluate(z)
                self.assertAlmostEqual(scoreonly / corr_score, 1, delta=1e-6)
                self.assertAlmostEqual(score / corr_score, 1, delta=1e-6)
                self.assertAlmostEqual(deriv / corr_derv, 1, delta=1e-6)


    def test_special_values(self):
        """Test special distance values are correctly calculated."""
        for i in range(10):
            N = random.randint(3, 10)
            b = random.uniform(.1, 5.)
            func = IMP.core.SurfaceTetheredChain(N, b)
            func.set_was_used(True)
            beta = _get_beta(N, b)
            zmin = 1. / (2 * beta)**.5
            zmean = .5 * (math.pi / beta)**.5
            self.assertAlmostEqual(func.get_distance_at_minimum(), zmin,
                                   delta=1e-6)
            self.assertAlmostEqual(func.evaluate_with_derivative(zmin)[1], 0.,
                                   delta=1e-6)
            self.assertAlmostEqual(func.get_average_distance(), zmean,
                                   delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
