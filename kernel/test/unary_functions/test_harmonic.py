import unittest
import IMP
import IMP.test

class HarmonicTests(IMP.test.TestCase):
    """Tests for harmonic unary function"""

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
