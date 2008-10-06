import unittest
import IMP
import IMP.test
import IMP.core

class LinearTests(IMP.test.TestCase):
    """Tests for linear unary function"""

    def test_values(self):
        """Test that linear values are correct"""
        for offset in (0.0, -1.0):
            for slope in (0.0, -5.0, 3.5):
                func = IMP.core.Linear(offset, slope)
                for i in range(15):
                    val = -10.0 + 3.5 * i
                    scoreonly = func.evaluate(val)
                    score, deriv = func.evaluate_with_derivative(val)
                    self.assertEqual(score, scoreonly)
                    self.assertInTolerance(score, (val - offset) * slope, 0.001)
                    self.assertInTolerance(deriv, slope, 0.001)

    def test_accessors(self):
        """Test Linear accessors"""
        func = IMP.core.Linear(0, 0)
        self.assertEqual(func.evaluate(1), 0)
        func.set_slope(10)
        self.assertEqual(func.evaluate(1), 10)
        func.set_offset(5)
        self.assertEqual(func.evaluate(1), -40)

    def test_show(self):
        """Check Linear::show() method"""
        func = IMP.core.Linear(0, 1.0)
        func.show()

if __name__ == '__main__':
    unittest.main()
