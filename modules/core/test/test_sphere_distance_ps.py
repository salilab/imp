import IMP
import IMP.test
import IMP.core
import pickle
try:
    import jax
except ImportError:
    jax = None


def make_score(uf):
    m = IMP.Model()
    s = IMP.core.SphereDistancePairScore(uf)
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4., 0., 0.), 1.0))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 6., 7.), 2.0))
    return m, p1, p2, s


def make_linear_score():
    return make_score(IMP.core.Linear(0, 1))


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of SphereDistancePairScore"""
        m, p1, p2, s = make_linear_score()
        s.set_name('foo')
        self.assertAlmostEqual(s.evaluate_index(m, (p1, p2), None),
                               6.2736, delta=1e-4)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(s.get_name(), 'foo')
        self.assertAlmostEqual(news.evaluate_index(m, (p1, p2), None),
                               6.2736, delta=1e-4)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SphereDistancePairScore via polymorphic ptr"""
        m, p1, p2, s = make_linear_score()
        r = IMP.core.PairRestraint(m, s, (p1, p2))
        self.assertAlmostEqual(r.evaluate(False), 6.2736, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 6.2736, delta=1e-4)

    def test_harmonic_upper_bound(self):
        """Test creation with HarmonicUpperBound"""
        # Should use HarmonicUpperBoundSphereDistancePairScore internally
        m, p1, p2, s = make_score(IMP.core.HarmonicUpperBound(2.0, 3.0))
        self.assertIsInstance(
            s._proxied, IMP.core.HarmonicUpperBoundSphereDistancePairScore)
        self.assertAlmostEqual(s.evaluate_index(m, [p1, p2], None), 27.3957,
                               delta=0.01)
        s2 = IMP.core.HarmonicUpperBoundSphereDistancePairScore(2.0, 3.0)
        self.assertAlmostEqual(s2.evaluate_index(m, [p1, p2], None), 27.3957,
                               delta=0.01)

        # Should be able to cast an object to either the
        # SphereDistancePairScore proxy or the underlying class
        obj = IMP.PairScore.get_from(s)
        s3 = IMP.core.SphereDistancePairScore.get_from(obj)
        self.assertIsInstance(
            s3._proxied, IMP.core.HarmonicUpperBoundSphereDistancePairScore)
        s3 = IMP.core.HarmonicUpperBoundSphereDistancePairScore.get_from(obj)
        self.assertIsInstance(
            s3, IMP.core.HarmonicUpperBoundSphereDistancePairScore)

    def test_harmonic(self):
        """Test creation with Harmonic"""
        # Should use HarmonicSphereDistancePairScore internally
        m, p1, p2, s = make_score(IMP.core.Harmonic(2.0, 3.0))
        self.assertIsInstance(
            s._proxied, IMP.core.HarmonicSphereDistancePairScore)
        self.assertAlmostEqual(s.evaluate_index(m, [p1, p2], None), 27.3957,
                               delta=0.01)
        s2 = IMP.core.HarmonicSphereDistancePairScore(2.0, 3.0)
        self.assertAlmostEqual(s2.evaluate_index(m, [p1, p2], None), 27.3957,
                               delta=0.01)

        # Should be able to cast an object to either the
        # SphereDistancePairScore proxy or the underlying class
        obj = IMP.PairScore.get_from(s)
        s3 = IMP.core.SphereDistancePairScore.get_from(obj)
        self.assertIsInstance(
            s3._proxied, IMP.core.HarmonicSphereDistancePairScore)
        s3 = IMP.core.HarmonicSphereDistancePairScore.get_from(obj)
        self.assertIsInstance(
            s3, IMP.core.HarmonicSphereDistancePairScore)

    def test_harmonic_lower_bound(self):
        """Test creation with HarmonicLowerBound"""
        # Should use SoftSpherePairScore internally if mean is zero,
        # otherwise the regular _SphereDistancePairScore
        m, p1, p2, s = make_score(IMP.core.HarmonicLowerBound(2.0, 3.0))
        self.assertIsInstance(
            s._proxied, IMP.core._SphereDistancePairScore)

        m, p1, p2, s = make_score(IMP.core.HarmonicLowerBound(0.0, 3.0))
        self.assertIsInstance(
            s._proxied, IMP.core.SoftSpherePairScore)
        IMP.core.XYZ(p2).set_coordinates(IMP.algebra.Vector3D(5,0,0))
        self.assertAlmostEqual(s.evaluate_index(m, [p1, p2], None), 6.0,
                               delta=0.01)
        s2 = IMP.core.SoftSpherePairScore(3.0)
        self.assertAlmostEqual(s2.evaluate_index(m, [p1, p2], None), 6.0,
                               delta=0.01)

        # Should be able to cast an object to either the
        # SphereDistancePairScore proxy or the underlying class
        obj = IMP.PairScore.get_from(s)
        s3 = IMP.core.SphereDistancePairScore.get_from(obj)
        self.assertIsInstance(s3._proxied, IMP.core.SoftSpherePairScore)
        s3 = IMP.core.SoftSpherePairScore.get_from(obj)
        self.assertIsInstance(s3, IMP.core.SoftSpherePairScore)

    def test_linear(self):
        """Test creation with Linear"""
        # Should use _SphereDistancePairScore internally
        m, p1, p2, s = make_score(IMP.core.Linear(2.0, 3.0))
        self.assertIsInstance(
            s._proxied, IMP.core._SphereDistancePairScore)
        self.assertAlmostEqual(s.evaluate_index(m, [p1, p2], None), 12.8209,
                               delta=0.01)
        s2 = IMP.core._SphereDistancePairScore(IMP.core.Linear(2.0, 3.0))
        self.assertAlmostEqual(s2.evaluate_index(m, [p1, p2], None), 12.8209,
                               delta=0.01)

        # Should be able to cast an object to either the
        # SphereDistancePairScore proxy or the underlying class
        obj = IMP.PairScore.get_from(s)
        s3 = IMP.core.SphereDistancePairScore.get_from(obj)
        self.assertIsInstance(s3._proxied, IMP.core._SphereDistancePairScore)
        s3 = IMP.core._SphereDistancePairScore.get_from(obj)
        self.assertIsInstance(s3, IMP.core._SphereDistancePairScore)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of _SphereDistancePairScore class"""
        import jax.numpy as jnp
        m, p1, p2, s = make_score(IMP.core.Linear(2.0, 3.0))
        ji = s._get_jax(m, jnp.array([[p1.get_index(), p2.get_index()]]))
        jax_s = jax.jit(ji.score_func)
        jm = ji.get_jax_model()
        imp_score_val = s.evaluate_index(m, (p1, p2), None)
        jax_score_val = jax_s(jm)
        self.assertAlmostEqual(imp_score_val, jax_score_val, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
