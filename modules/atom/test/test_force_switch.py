import IMP
import IMP.test
import IMP.atom
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    """Test ForceSwitch"""

    def test_create(self):
        """Check ForceSwitch creation"""
        s = IMP.atom.ForceSwitch(5.0, 6.0)
        self.assertAlmostEqual(s.get_min_distance(), 5.0, delta=1e-5)
        self.assertAlmostEqual(s.get_max_distance(), 6.0, delta=1e-5)
        self.assertRaisesUsageException(IMP.atom.ForceSwitch, 6.0, 5.0)

    def test_standard_object_methods(self):
        """Check ForceSwitch standard object methods"""
        sm = IMP.atom.ForceSwitch(9.0, 10.0)
        self.check_standard_object_methods(sm)

    def test_value(self):
        """Check value of ForceSwitch"""
        s = IMP.atom.ForceSwitch(5.0, 6.0)

        # dist < min_distance should be unchanged
        for dist in (0, 1, 2, 3, 4, 5):
            self.assertEqual(s(100.0, dist), 100.0)
            self.assertEqual(s(100.0, 0.0, dist)[0], 100.0)
        # dist > min_distance should be zero
        for dist in (7, 8, 9, 10):
            self.assertEqual(s(100.0, dist), 0.0)
            self.assertEqual(s(100.0, 0.0, dist)[0], 0.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Check JAX implementation"""
        s = IMP.atom.ForceSwitch(5.0, 6.0)
        j = jax.jit(s._get_jax())
        for dist in (0., 1., 2., 4., 5.1, 5.2, 5.7, 6.0, 6.1, 7.0):
            self.assertAlmostEqual(s(100.0, dist), j(100.0, dist), delta=1e-4)

    def test_deriv(self):
        """Check derivatives of ForceSwitch"""
        s = IMP.atom.ForceSwitch(3.0, 6.0)

        def func(val):
            return s(100.0, val)

        for dist in range(10):
            score, deriv = s(100.0, 0.0, dist)
            num_deriv = IMP.test.numerical_derivative(func, dist, 0.01)
            self.assertAlmostEqual(deriv, num_deriv, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
