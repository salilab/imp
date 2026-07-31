import math
import IMP
import IMP.test
import IMP.core
try:
    import jax
except ImportError:
    jax = None


def _sum_of_exponent(fs, weights, x, d=1.):
    exp_sum = 0.
    for w, f in zip(weights, fs):
        exp_sum += w * math.exp(-f.evaluate(x) / d)
    return -math.log(exp_sum) * d


def _derv_sum_of_exponent(fs, weights, x, d=1.):
    derv_num = 0.
    exp_sum = 0.
    for w, f in zip(weights, fs):
        val, derv = f.evaluate_with_derivative(x)
        exp_sum += w * math.exp(-val / d)
        derv_num += w * math.exp(-val / d) * derv
    return derv_num / exp_sum


class Tests(IMP.test.TestCase):

    def test_values(self):
        """Check value of WeightedSumOfExponential function"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSumOfExponential([f1, f2], [.3, .7])
        for i in range(-10, 10):
            i = float(i)
            exp_score = _sum_of_exponent([f1, f2], [.3, .7], i)
            self.assertAlmostEqual(sf.evaluate(i), exp_score, delta=1e-6)
            exp_score = _sum_of_exponent([f1, f2], [.3, .7], i)
            exp_deriv = _derv_sum_of_exponent([f1, f2], [.3, .7], i)
            score, deriv = sf.evaluate_with_derivative(i)
            self.assertAlmostEqual(score, exp_score, delta=1e-6)
            self.assertAlmostEqual(deriv, exp_deriv, delta=1e-6)

        sf = IMP.core.WeightedSumOfExponential([f1, f2], [.3, .7], 20.)
        for i in range(-10, 10):
            i = float(i)
            exp_score = _sum_of_exponent([f1, f2], [.3, .7], i, d=20.)
            self.assertAlmostEqual(sf.evaluate(i), exp_score, delta=1e-4)
            exp_score = _sum_of_exponent([f1, f2], [.3, .7], i, d=20.)
            exp_deriv = _derv_sum_of_exponent([f1, f2], [.3, .7], i, d=20.)
            score, deriv = sf.evaluate_with_derivative(i)
            self.assertAlmostEqual(score, exp_score, delta=1e-4)
            self.assertAlmostEqual(deriv, exp_deriv, delta=1e-4)

    def test_update_functions(self):
        """Test changing WeightedSumOfExponential parameters"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 2.)
        sf = IMP.core.WeightedSumOfExponential([f1, f2], [.5, .5])
        self.assertAlmostEqual(sf.evaluate(0), .674997, delta=1e-6)
        f2.set_k(1.)
        self.assertAlmostEqual(sf.evaluate(0), .566219, delta=1e-6)

    def test_accessors(self):
        """Test WeightedSumOfExponential accessors"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSumOfExponential([f1, f2], [.3, .7], 2.)
        self.assertAlmostEqual(sf.get_weight(0), .3)
        self.assertAlmostEqual(sf.get_weight(1), .7)
        self.assertAlmostEqual(sf.get_weights()[0], .3)
        sf.set_weights([.4, .6])
        self.assertAlmostEqual(sf.get_weight(0), .4)
        self.assertAlmostEqual(sf.get_weight(1), .6)
        self.assertAlmostEqual(sf.get_denominator(), 2.)
        sf.set_denominator(3.)
        self.assertAlmostEqual(sf.get_denominator(), 3.)

    def test_errors(self):
        """Test handling of incorrect inputs to WeightedSumOfExponential"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        self.assertRaisesUsageException(IMP.core.WeightedSumOfExponential,
                                        [f1], [1.])
        self.assertRaisesUsageException(IMP.core.WeightedSumOfExponential,
                                        [f1, f2], [1.])
        self.assertRaisesUsageException(IMP.core.WeightedSumOfExponential,
                                        [f1, f2], [1.], .0)
        sf = IMP.core.WeightedSumOfExponential([f1, f2], [.3, .7])
        self.assertRaisesUsageException(sf.set_weights, [1.])
        self.assertRaisesUsageException(sf.set_denominator, 0.)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of WeightedSumOfExponential"""
        import jax.numpy as jnp
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSumOfExponential([f1, f2], [.3, .7])
        jsf = jax.jit(sf._get_jax())
        imp_score = sf.evaluate(4.0)
        jax_score = jsf(4.0)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-3)
        # Should also work if given an array
        vals = jnp.array([4.0, 6.0])
        scores = jsf(vals)
        self.assertEqual(scores.shape, (2,))
        self.assertAlmostEqual(scores[0], 6.300, delta=1e-3)
        self.assertAlmostEqual(scores[0], jsf(vals[0]), delta=1e-3)
        self.assertAlmostEqual(scores[1], 19.198, delta=1e-3)
        self.assertAlmostEqual(scores[1], jsf(vals[1]), delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
