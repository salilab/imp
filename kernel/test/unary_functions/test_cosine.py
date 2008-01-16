import unittest
import IMP
import IMP.test
import math

def _cosfunc(val, force_constant, periodicity, phase):
    """Python implementation of cosine function and first derivative"""
    score = abs(force_constant) \
            - force_constant * math.cos(periodicity * val + phase)
    deriv = force_constant * periodicity * math.sin(periodicity * val + phase)
    return score, deriv

class CosineTests(IMP.test.TestCase):
    """Tests for cosine unary function"""

    def test_values(self):
        """Test that cosine values are correct"""
        for force_constant in (0, 10.0, -5.0):
            for periodicity in (1, 2, 3, 4):
                for phase in (0.0, math.pi / 2.0, math.pi):
                    func = IMP.Cosine(force_constant, periodicity, phase)
                    for i in range(15):
                        val = -math.pi + math.pi * 15.0 / (i + 1.0)
                        diff = func(val) - _cosfunc(val, force_constant,
                                                    periodicity, phase)[0]
                        self.assert_(abs(diff) < 0.1)

    def test_show(self):
        """Check Cosine::show() method"""
        func = IMP.Cosine(10.0, 1, 0.0)
        func.show()

if __name__ == '__main__':
    unittest.main()
