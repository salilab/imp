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
    s = IMP.core.SoftSpherePairScore(1)
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4., 0., 0.), 1.0))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 1., 0.), 2.0))
    return m, p1, p2, s


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of SoftSpherePairScore"""
        m, p1, p2, s = make_score()
        s.set_name('foo')
        self.assertAlmostEqual(s.evaluate_index(m, (p1, p2), None),
                               1.2574, delta=1e-4)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(s.get_name(), 'foo')
        self.assertAlmostEqual(news.evaluate_index(m, (p1, p2), None),
                               1.2574, delta=1e-4)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SoftSpherePairScore via poly ptr"""
        m, p1, p2, s = make_score()
        r = IMP.core.PairRestraint(m, s, (p1, p2))
        self.assertAlmostEqual(r.evaluate(False), 1.2574, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 1.2574, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of SoftSpherePairScore"""
        import IMP._jax_util
        m, p1, p2, s = make_score()
        r = IMP.core.PairRestraint(m, s, (p1, p2))
        imp_score = r.evaluate(False)
        ji = r._get_jax(space=IMP._jax_util.FreeSpace)
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        # Check score with overlapping spheres
        self.assertAlmostEqual(imp_score, 1.25736, delta=1e-3)
        self.assertAlmostEqual(imp_score, j(jm), delta=1e-3)
        # Check score with non-overlapping spheres
        IMP.core.XYZ(p2).set_coordinates(IMP.algebra.Vector3D(10., 1., 0.))
        imp_score = r.evaluate(False)
        jm = ji.get_jax_model()
        self.assertAlmostEqual(imp_score, 0.0, delta=1e-3)
        self.assertAlmostEqual(imp_score, j(jm), delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_periodic(self):
        """Test JAX implementation of SoftSpherePairScore with PBC"""
        import jax.numpy as jnp
        import IMP._jax_util
        space = IMP._jax_util.PeriodicSpace([1., 1., 1.])
        m, p1, p2, s = make_score()
        ji = s._get_jax(m, jnp.array([[p1.get_index(), p2.get_index()]]),
                        space=space)
        jax_s = jax.jit(ji.score_func)
        jm = ji.get_jax_model()
        jax_score = jax_s(jm)
        self.assertAlmostEqual(jax_score, 4.5, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
