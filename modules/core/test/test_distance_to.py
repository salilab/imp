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
    func = IMP.core.Harmonic(1.0, 4.0)
    s = IMP.core.DistanceToSingletonScore(func, IMP.algebra.Vector3D(0,0,0))
    p = IMP.Particle(m)
    IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(4,0,0))
    return m, p, s


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of DistanceToSingletonScore"""
        m, p, s = make_score()
        s.set_name('foo')
        self.assertAlmostEqual(s.evaluate_index(m, p, None), 18.0, delta=0.01)
        dump = pickle.dumps(s)
        news = pickle.loads(dump)
        self.assertEqual(s.get_name(), 'foo')
        self.assertAlmostEqual(news.evaluate_index(m, p, None),
                               18.0, delta=0.01)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of DistanceToSingletonScore via polymorphic ptr"""
        m, p, s = make_score()
        r = IMP.core.SingletonRestraint(m, s, p)
        self.assertAlmostEqual(r.evaluate(False), 18.0, delta=0.01)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 18.0, delta=0.01)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_container(self):
        """Test JAX DistanceToSingletonScore in a container"""
        m, p, s = make_score()
        lsc = IMP.container.ListSingletonContainer(m)
        lsc.add([p])
        r = IMP.container.SingletonsRestraint(s, lsc)
        sf = IMP.core.RestraintsScoringFunction([r])

        ji = sf._get_jax()
        X = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        # Compare JAX with IMP C++ implementation
        self.assertAlmostEqual(j(X), sf.evaluate(False), delta=0.01)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_restraint(self):
        """Test JAX DistanceToSingletonScore in a SingletonRestraint"""
        m, p, s = make_score()
        r = IMP.core.SingletonRestraint(m, s, p)

        ji = r._get_jax()
        X = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        # Compare JAX with IMP C++ implementation
        self.assertAlmostEqual(j(X), r.evaluate(False), delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
