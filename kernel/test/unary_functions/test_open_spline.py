import unittest
import IMP
import IMP.test
import IMP.utils
import math

def _testfunc(val):
    """Simple function and its first derivative"""
    return 2.0 + val * val + 3.0 * val + 1.0, 4.0 * val + 3.0;

class OpenCubicSplineTests(IMP.test.TestCase):
    """Tests for open cubic spline unary functions"""

    def test_range(self):
        """Test for spline out of range condition"""
        floats = IMP.Floats()
        floats.append(0.)
        floats.append(0.)
        spline = IMP.OpenCubicSpline(floats, 10.0, 10.0)
        self.assertEqual(spline(10.0), 0.0)
        self.assertEqual(spline(20.0), 0.0)
        self.assertRaises(ValueError, spline, 9.9)
        self.assertRaises(ValueError, spline, 20.1)

    def test_interpolate(self):
        """Test that spline-interpolated values are correct"""
        floats = IMP.Floats()
        # Build spline using test function values:
        minrange = 4.0
        spacing = 1.0
        for i in range(10):
            floats.append(_testfunc(minrange + spacing * i)[0])
        spline = IMP.OpenCubicSpline(floats, minrange, spacing)

        # Now test the spline against the test function for intermediate points
        spacing = 0.15
        # todo: also test derivatives: but this will have to wait for
        # operator()(Float, Float &) support in the Python interface
        for i in range(30):
            val = minrange + spacing * i
            diff = spline(val) - _testfunc(val)[0]
            self.assert_(abs(diff) < 0.1)

    def test_show(self):
        """Check OpenCubicSpline::show() method"""
        floats = IMP.Floats()
        floats.append(0.)
        floats.append(0.)
        spline = IMP.OpenCubicSpline(floats, 10.0, 10.0)
        spline.show()

if __name__ == '__main__':
    unittest.main()
