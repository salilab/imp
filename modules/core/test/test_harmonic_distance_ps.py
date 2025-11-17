import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle
try:
    import jax
except ImportError:
    jax = None

def make_score():
    m = IMP.Model()
    s = IMP.core.HarmonicDistancePairScore(0, 1)
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4., 0., 0.), 1.0))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 6., 7.), 2.0))
    return m, p1, p2, s


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of HarmonicDistancePairScore"""
        m, p1, p2, s = make_score()
        s.set_name('foo')
        self.assertAlmostEqual(s.evaluate_index(m, (p1, p2), None),
                               43.0, delta=1e-4)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(s.get_name(), 'foo')
        self.assertAlmostEqual(news.evaluate_index(m, (p1, p2), None),
                               43.0, delta=1e-4)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of HarmonicDistancePairScore via poly ptr"""
        m, p1, p2, s = make_score()
        r = IMP.core.PairRestraint(m, s, (p1, p2))
        self.assertAlmostEqual(r.evaluate(False), 43.0, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 43.0, delta=1e-4)

    def test_get_derived_object(self):
        """Test cast to HarmonicDistancePairScore"""
        m, p1, p2, s = make_score()
        r = IMP.container.PairsRestraint(s, [(p1, p2)])
        new_s = r.get_score_object()
        self.assertIs(type(new_s), IMP.PairScore)
        der_new_s = new_s.get_derived_object()
        self.assertIsInstance(der_new_s, IMP.core.HarmonicDistancePairScore)

    def test_accessors(self):
        """Test HarmonicDistancePairScore accessors"""
        m, p1, p2, s = make_score()
        self.assertAlmostEqual(s.get_x0(), 0.0, delta=1e-5)
        self.assertAlmostEqual(s.get_k(), 1.0, delta=1e-5)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation"""
        import jax.numpy as jnp
        m, p1, p2, s = make_score()
        ji = s._get_jax()
        jax_s = jax.jit(ji.score_func)
        ji.m = m
        X = ji.get_model_state()
        imp_score_val = s.evaluate_index(m, (p1, p2), None)
        jax_score_val = jax_s(X,
                              jnp.array([[p1.get_index(), p2.get_index()]]))
        self.assertAlmostEqual(imp_score_val, jax_score_val, delta=1e-5)

        # Test score inside RestraintsScoringFunction
        lpc = IMP.container.ListPairContainer(m)
        lpc.add((p1, p2))
        sf = IMP.core.RestraintsScoringFunction(
            [IMP.container.PairsRestraint(s, lpc)])
        ji = sf._get_jax()
        jax_s = jax.jit(ji.score_func)
        X = ji.get_model_state()
        jax_score_val = jax_s(X)
        self.assertAlmostEqual(imp_score_val, jax_score_val, delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
