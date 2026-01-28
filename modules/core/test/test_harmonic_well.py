import IMP
import IMP.test
import IMP.core
import pickle
try:
    import jax
except ImportError:
    jax = None


def _harmonic_well_func(val, lb, ub, force_constant):
    """Python implementation of harmonic well function and 1st deriv"""
    if val < lb:
        diff = val - lb
    elif val > ub:
        diff = val - ub
    else:
        return 0., 0.
    score = 0.5 * force_constant * diff * diff
    deriv = force_constant * diff
    return score, deriv


class Tests(IMP.test.TestCase):

    def test_values(self):
        """Test that harmonic well values are correct"""
        force_constant = 100.0
        lb = 8.0
        ub = 11.0
        func = IMP.core.HarmonicWell((lb, ub), force_constant)
        func.set_was_used(True)
        for i in range(15):
            val = 5.0 + i
            expscore, expderiv = _harmonic_well_func(val, lb, ub,
                                                     force_constant)
            score, deriv = func.evaluate_with_derivative(val)
            scoreonly = func.evaluate(val)
            self.assertEqual(score, scoreonly)
            self.assertAlmostEqual(expscore, score, delta=0.1)
            self.assertAlmostEqual(expderiv, deriv, delta=0.1)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test that JAX harmonic well values are correct"""
        force_constant = 100.0
        lb = 8.0
        ub = 11.0
        func = IMP.core.HarmonicWell((lb, ub), force_constant)
        func.set_was_used(True)
        score_func = func._get_jax()
        score_f = jax.jit(score_func)
        deriv_f = jax.jit(jax.grad(score_func))
        for i in range(15):
            val = 5.0 + i
            expscore, expderiv = _harmonic_well_func(val, lb, ub,
                                                     force_constant)
            score = score_f(val)
            deriv = deriv_f(val)
            self.assertAlmostEqual(expscore, score, delta=0.1)
            self.assertAlmostEqual(expderiv, deriv, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
