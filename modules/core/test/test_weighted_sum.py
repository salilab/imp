import IMP
import IMP.test
import IMP.core
import IMP.container
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def test_values(self):
        """Check value of WeightedSum function"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        for i in range(-10, 10):
            i = float(i)
            self.assertAlmostEqual(
                sf.evaluate(i), .3 * f1.evaluate(i) + .7 * f2.evaluate(i))
            score, deriv = sf.evaluate_with_derivative(i)
            score_sum = 0
            deriv_sum = 0
            for w, f in zip([.3, .7], [f1, f2]):
                s, d = f.evaluate_with_derivative(i)
                score_sum += w * s
                deriv_sum += w * d
            self.assertAlmostEqual(score, score_sum, delta=1e-4)
            self.assertAlmostEqual(deriv, deriv_sum, delta=1e-4)

    def test_update_functions(self):
        """Test changing WeightedSum parameters"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 2.)
        sf = IMP.core.WeightedSum([f1, f2], [.5, .5])
        self.assertAlmostEqual(sf.evaluate(0), 2., delta=1e-6)
        f2.set_k(1.)
        self.assertAlmostEqual(sf.evaluate(0), 1., delta=1e-6)

    def test_accessors(self):
        """Test WeightedSum accessors"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        self.assertAlmostEqual(sf.get_weight(0), .3)
        self.assertAlmostEqual(sf.get_weight(1), .7)
        self.assertAlmostEqual(sf.get_weights()[0], .3)
        sf.set_weights([.4, .6])
        self.assertAlmostEqual(sf.get_weight(0), .4)
        self.assertAlmostEqual(sf.get_weight(1), .6)

    def test_errors(self):
        """Test handling of incorrect inputs to WeightedSum"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        self.assertRaisesUsageException(IMP.core.WeightedSum,
                                        [f1], [1.])
        self.assertRaisesUsageException(IMP.core.WeightedSum,
                                        [f1, f2], [1.])
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        self.assertRaisesUsageException(sf.set_weights,
                                        [1.])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_single(self):
        """Test JAX implementation of WeightedSum, single score"""
        import jax.numpy as jnp
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        jsf = jax.jit(sf._get_jax())
        imp_score = sf.evaluate(4.0)
        jax_score = jsf(4.0)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-3)
        # Should also work if given an array
        vals = jnp.array([4.0, 6.0])
        scores = jsf(vals)
        self.assertEqual(scores.shape, (2,))
        self.assertAlmostEqual(scores[0], 6.6, delta=1e-3)
        self.assertAlmostEqual(scores[0], jsf(vals[0]), delta=1e-3)
        self.assertAlmostEqual(scores[1], 22.2, delta=1e-3)
        self.assertAlmostEqual(scores[1], jsf(vals[1]), delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multiple(self):
        """Test JAX implementation of WeightedSum, multiple scores"""
        f1 = IMP.core.Harmonic(0., 1.)
        f2 = IMP.core.Harmonic(2., 3.)
        sf = IMP.core.WeightedSum([f1, f2], [.3, .7])
        m = IMP.Model()
        p1 = self.create_point_particle(m, -3.0, 0.0, 0.0)
        p2 = self.create_point_particle(m, 4.0, 0.0, 0.0)
        ss = IMP.core.DistanceToSingletonScore(
            sf, IMP.algebra.Vector3D(0., 0., 0.))
        lsc = IMP.container.ListSingletonContainer(m, [p1, p2])
        r = IMP.container.SingletonsRestraint(ss, lsc)
        imp_score = r.evaluate(False)
        ji = r._get_jax()
        jm = ji.get_jax_model()
        jsf = jax.jit(ji.score_func)
        jax_score = jsf(jm)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
