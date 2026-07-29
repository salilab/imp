import IMP
import IMP.test
import IMP.core
import pickle
try:
    import jax
except ImportError:
    jax = None


def make_score():
    m = IMP.Model()
    s = IMP.core.HarmonicSphereDistancePairScore(0, 1)
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4., 0., 0.), 1.0))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 6., 7.), 2.0))
    return m, p1, p2, s


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of HarmonicSphereDistancePairScore"""
        m, p1, p2, s = make_score()
        s.set_name('foo')
        self.assertAlmostEqual(s.evaluate_index(m, (p1, p2), None),
                               19.6791, delta=1e-4)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(s.get_name(), 'foo')
        self.assertAlmostEqual(news.evaluate_index(m, (p1, p2), None),
                               19.6791, delta=1e-4)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of HarmonicSphereDistancePairScore via poly ptr"""
        m, p1, p2, s = make_score()
        r = IMP.core.PairRestraint(m, s, (p1, p2))
        self.assertAlmostEqual(r.evaluate(False), 19.6791, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 19.6791, delta=1e-4)

    def test_accessors(self):
        """Test HarmonicSphereDistancePairScore accessors"""
        m, p1, p2, s = make_score()
        self.assertAlmostEqual(s.get_x0(), 0.0, delta=1e-5)
        self.assertAlmostEqual(s.get_k(), 1.0, delta=1e-5)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation"""
        import jax.numpy as jnp
        m, p1, p2, s = make_score()
        ji = s._get_jax(m, jnp.array([[p1.get_index(), p2.get_index()]]))
        jax_s = jax.jit(ji.score_func)
        X = ji.get_jax_model()
        imp_score_val = s.evaluate_index(m, (p1, p2), None)
        jax_score_val = jax_s(X)
        self.assertAlmostEqual(imp_score_val, jax_score_val, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
