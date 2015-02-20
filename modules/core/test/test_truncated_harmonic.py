from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import random


class Tests(IMP.test.TestCase):

    """Tests for linear unary function"""
    nreps = 10

    def _test_one(self, f, m):
        self.check_unary_function_deriv(f, -30, 30, .1)

    def test_lvalues(self):
        """Check TruncatedHarmonicLowerBound"""
        for i in range(self.nreps):
            c = random.uniform(-10, 10)
            k = random.uniform(.1, 10)
            t = random.uniform(0, 10)
            l = random.uniform(1.1 * .5 * k * t * t, 10 + 1.1 * .5 * k * t * t)
            h = IMP.core.TruncatedHarmonicLowerBound(c, k, t, l)
            self.check_unary_function_deriv(h, -10, 10, .1)
            self.assertEqual(h.evaluate(c + 1), 0)
            self.assertAlmostEqual(h.evaluate(c - t - 100000), l, delta=.1)

    def test_uvalues(self):
        """Check TruncatedHarmonicUpperBound"""
        for i in range(self.nreps):
            c = random.uniform(-10, 10)
            k = random.uniform(.1, 10)
            t = random.uniform(0, 10)
            l = random.uniform(1.1 * .5 * k * t * t, 10 + 1.1 * .5 * k * t * t)
            c = 0
            k = 1
            t = 2
            l = 10
            h = IMP.core.TruncatedHarmonicUpperBound(c, k, t, l)
            self.check_unary_function_deriv(h, -10, 10, .1)
            self.assertEqual(h.evaluate(c - 1), 0)
            self.assertAlmostEqual(h.evaluate(c + t + 100000), l, delta=.1)

    def test_values(self):
        """Check TruncatedHarmonicBound"""
        for i in range(self.nreps):
            c = random.uniform(-10, 10)
            k = random.uniform(.1, 10)
            t = random.uniform(0, 10)
            l = random.uniform(1.1 * .5 * k * t * t, 10 + 1.1 * .5 * k * t * t)
            h = IMP.core.TruncatedHarmonicBound(c, k, t, l)

            self.assertAlmostEqual(h.evaluate(c + t + 100000), l, delta=.1)
            self.assertAlmostEqual(h.evaluate(c - t - 100000), l, delta=.1)
            self.check_unary_function_min(h, -10, 10, .1, c)
            self.check_unary_function_deriv(h, -10, 10, .1)

if __name__ == '__main__':
    IMP.test.main()
