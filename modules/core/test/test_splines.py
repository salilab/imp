import IMP
import IMP.test
import IMP.core
import math
try:
    import jax
except ImportError:
    jax = None


def _testfunc(val):
    """Simple function and its first derivative"""
    return val * val + 3.0 * val + 1.0, 2.0 * val + 3.0


def _periodic_testfunc(val):
    """Simple periodic function and its first derivative"""
    return math.cos(val), -math.sin(val)


class Tests(IMP.test.TestCase):

    """Tests for cubic spline unary functions"""

    def test_range(self):
        """Test for spline out of range condition"""
        floats = []
        floats.append(0.)
        floats.append(0.)
        floats.append(0.)
        open_spline = IMP.core.OpenCubicSpline(floats, 10.0, 5.0)
        closed_spline = IMP.core.ClosedCubicSpline(floats, 10.0, 5.0)
        open_spline.set_was_used(True)
        closed_spline.set_was_used(True)
        self.assertEqual(open_spline.evaluate(10.0), 0.0)
        self.assertEqual(open_spline.evaluate(20.0), 0.0)
        self.assertRaises(ValueError, open_spline.evaluate, 9.9)
        self.assertRaises(ValueError, open_spline.evaluate, 20.1)
        self.assertEqual(closed_spline.evaluate(10.0), 0.0)
        self.assertEqual(closed_spline.evaluate(25.0), 0.0)
        self.assertRaises(ValueError, closed_spline.evaluate, 9.9)
        self.assertRaises(ValueError, closed_spline.evaluate, 25.1)

    def test_interpolate(self):
        """Test that spline-interpolated values are correct"""
        self.__interpolate(minrange=4.0, spline_spacing=1.0, test_spacing=0.15,
                           test_func=_testfunc,
                           spline_func=IMP.core.OpenCubicSpline)
        self.__interpolate(minrange=0.0, spline_spacing=math.pi / 5.0,
                           test_spacing=math.pi / 15.0,
                           test_func=_periodic_testfunc,
                           spline_func=IMP.core.ClosedCubicSpline)

    def __interpolate(self, minrange, spline_spacing, test_spacing, test_func,
                      spline_func):
        floats = []
        # Build spline using test function values:
        for i in range(10):
            floats.append(test_func(minrange + spline_spacing * i)[0])
        spline = spline_func(floats, minrange, spline_spacing)
        spline.set_was_used(True)

        # Now test the spline against the test function for intermediate points
        for i in range(30):
            val = minrange + test_spacing * i
            scoreonly = spline.evaluate(val)
            score, deriv = spline.evaluate_with_derivative(val)
            self.assertAlmostEqual(score, scoreonly, delta=.01)
            expscore, expderiv = test_func(val)
            self.assertAlmostEqual(score, expscore, delta=0.1)
            self.assertAlmostEqual(deriv, expderiv, delta=0.6)

    def test_show(self):
        """Check cubic spline show() methods"""
        floats = []
        floats.append(0.)
        floats.append(0.)
        floats.append(0.)
        open_spline = IMP.core.OpenCubicSpline(floats, 10.0, 10.0)
        closed_spline = IMP.core.ClosedCubicSpline(floats, 10.0, 10.0)
        open_spline.set_was_used(True)
        closed_spline.set_was_used(False)
        open_spline.show()
        closed_spline.show()

    def test_closed_accessors(self):
        """Test ClosedCubicSpline accessors"""
        s = IMP.core.ClosedCubicSpline([1.0, 2.0, 4.0], 4.0, 10.0)
        self.assertAlmostEqual(s.get_minrange(), 4.0, delta=1e-4)
        self.assertAlmostEqual(s.get_spacing(), 10.0, delta=1e-4)
        v = s.get_values()
        self.assertEqual(len(v), 3)
        self.assertAlmostEqual(v[0], 1.0, delta=1e-4)
        v2 = s.get_second_derivatives()
        self.assertEqual(len(v2), 3)

    def test_open_accessors(self):
        """Test OpenCubicSpline accessors"""
        s = IMP.core.OpenCubicSpline([1.0, 2.0, 4.0], 4.0, 10.0, extend=True)
        self.assertAlmostEqual(s.get_minrange(), 4.0, delta=1e-4)
        self.assertAlmostEqual(s.get_spacing(), 10.0, delta=1e-4)
        self.assertTrue(s.get_extend())
        v = s.get_values()
        self.assertEqual(len(v), 3)
        self.assertAlmostEqual(v[0], 1.0, delta=1e-4)
        v2 = s.get_second_derivatives()
        self.assertEqual(len(v2), 3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_closed_jax(self):
        """Test JAX implementation of ClosedCubicSpline"""
        import jax
        import jax.numpy as jnp
        s = IMP.core.ClosedCubicSpline([1.0, 2.0, 4.0], 10.0, 2.0)
        f = jax.jit(s._get_jax())
        vals = [10.2, 12.2, 15.9]
        for val in vals:
            self.assertAlmostEqual(s.evaluate(val), f(val), delta=1e-3)
        # Check given array as input
        fs = f(jnp.asarray(vals))
        self.assertEqual(len(fs), 3)
        self.assertAlmostEqual(fs[0], f(vals[0]), delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
