import IMP
import IMP.test
import IMP.core
import pickle
try:
    import jax
except ImportError:
    jax = None


def _ub_harmonicfunc(val, mean, force_constant):
    """Python implementation of upper bound harmonic function and 1st deriv"""
    if val < mean:
        return 0., 0.
    else:
        diff = val - mean
        score = 0.5 * force_constant * diff * diff
        deriv = force_constant * diff
        return score, deriv


class Tests(IMP.test.TestCase):

    def test_values(self):
        """Test that harmonic upper bound values are correct"""
        force_constant = 100.0
        mean = 10.0
        func = IMP.core.HarmonicUpperBound(mean, force_constant)
        func.set_was_used(True)
        for i in range(15):
            val = 5.0 + i
            expscore, expderiv = _ub_harmonicfunc(val, mean, force_constant)
            score, deriv = func.evaluate_with_derivative(val)
            scoreonly = func.evaluate(val)
            self.assertEqual(score, scoreonly)
            self.assertAlmostEqual(expscore, score, delta=0.1)
            self.assertAlmostEqual(expderiv, deriv, delta=0.1)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test that JAX harmonic upper bound values are correct"""
        force_constant = 100.0
        mean = 10.0
        func = IMP.core.HarmonicUpperBound(mean, force_constant)
        func.set_was_used(True)
        score_func = func._get_jax()
        score_f = jax.jit(score_func)
        deriv_f = jax.jit(jax.grad(score_func))
        for i in range(15):
            val = 5.0 + i
            expscore, expderiv = _ub_harmonicfunc(val, mean, force_constant)
            score = score_f(val)
            deriv = deriv_f(val)
            self.assertAlmostEqual(expscore, score, delta=0.1)
            self.assertAlmostEqual(expderiv, deriv, delta=0.1)

    def test_pickle(self):
        """Test (un-)pickle of HarmonicUpperBound"""
        func = IMP.core.HarmonicUpperBound(1.0, 4.0)
        func.set_name('foo')
        self.assertAlmostEqual(func.evaluate(4.0), 18.0, delta=0.01)
        dump = pickle.dumps(func)
        del func
        f = pickle.loads(dump)
        self.assertEqual(f.get_name(), 'foo')
        self.assertAlmostEqual(f.evaluate(4.0), 18.0, delta=0.01)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of HarmonicUpperBound via polymorphic pointer"""
        m = IMP.Model()
        func = IMP.core.HarmonicUpperBound(1.0, 4.0)
        s = IMP.core.DistanceToSingletonScore(func, IMP.algebra.Vector3D(0,0,0))
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(4,0,0))
        self.assertAlmostEqual(s.evaluate_index(m, p, None), 18.0, delta=0.01)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertAlmostEqual(news.evaluate_index(m, p, None),
                               18.0, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
