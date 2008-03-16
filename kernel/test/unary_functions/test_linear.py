import unittest
import IMP
import IMP.test

class LinearTests(IMP.test.TestCase):
    """Tests for linear unary function"""

    def test_values(self):
        """Test that linear values are correct"""
        for slope in (0.0, -5.0, 3.5):
            func = IMP.Linear(slope)
            for i in range(15):
                val = -10.0 + 3.5 * i
                scoreonly = func.evaluate(val)
                score, deriv = func.evaluate_deriv(val)
                self.assertEqual(score, scoreonly)
                self.assertInTolerance(score, val * slope, 0.001)
                self.assertInTolerance(deriv, slope, 0.001)

    def test_show(self):
        """Check Linear::show() method"""
        func = IMP.Linear(1.0)
        func.show()

if __name__ == '__main__':
    unittest.main()
