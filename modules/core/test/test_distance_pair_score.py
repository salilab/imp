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
    s = IMP.core.DistancePairScore(IMP.core.Linear(0, 1))
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4., 0., 0.), 1.0))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 6., 7.), 2.0))
    return m, p1, p2, s


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of DistancePairScore"""
        m, p1, p2, s = make_score()
        s.set_name('foo')
        self.assertAlmostEqual(s.evaluate_index(m, (p1, p2), None),
                               9.2736, delta=1e-4)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(s.get_name(), 'foo')
        self.assertAlmostEqual(news.evaluate_index(m, (p1, p2), None),
                               9.2736, delta=1e-4)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of DistancePairScore via polymorphic ptr"""
        m, p1, p2, s = make_score()
        r = IMP.core.PairRestraint(m, s, (p1, p2))
        self.assertAlmostEqual(r.evaluate(False), 9.2736, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 9.2736, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of DistancePairScore"""
        import jax.numpy as jnp
        m, p1, p2, s = make_score()
        imp_score = s.evaluate_index(m, (p1, p2), None)
        ji = s._get_jax(m, jnp.array([[p1.get_index(), p2.get_index()]]))
        jm = ji.get_jax_model()
        f = jax.jit(ji.score_func)
        jax_score = f(jm)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
