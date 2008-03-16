import unittest
import IMP
import IMP.test

def _harmonicfunc(val, mean, force_constant):
    """Python implementation of harmonic function and first derivative"""
    diff = val - mean
    score = 0.5 * force_constant * diff * diff
    deriv = force_constant * diff
    return score, deriv


class HarmonicTests(IMP.test.TestCase):
    """Tests for harmonic unary function"""

    def test_values(self):
        """Test that harmonic values are correct"""
        force_constant = 100.0
        mean = 10.0
        func = IMP.Harmonic(mean, force_constant)
        for i in range(15):
            val = 5.0 + i
            expscore, expderiv = _harmonicfunc(val, mean, force_constant)
            score, deriv = func.evaluate_deriv(val)
            scoreonly = func.evaluate(val)
            self.assertEqual(score, scoreonly)
            self.assertInTolerance(expscore, score, 0.1)
            self.assertInTolerance(expderiv, deriv, 0.1)

    def test_accessors(self):
        """Test Harmonic accessors"""
        func = IMP.Harmonic(10.0, 1.0)
        self.assertEqual(func.get_mean(), 10.0)
        self.assertEqual(func.get_k(), 1.0)
        func.set_mean(5.0)
        func.set_k(2.0)
        self.assertEqual(func.get_mean(), 5.0)
        self.assertEqual(func.get_k(), 2.0)

    def test_show(self):
        """Check Harmonic::show() method"""
        for func in (IMP.Harmonic(10.0, 1.0), IMP.HarmonicLowerBound(10.0, 1.0),
                     IMP.HarmonicUpperBound(10.0, 1.0)):
            func.show()

if __name__ == '__main__':
    unittest.main()
