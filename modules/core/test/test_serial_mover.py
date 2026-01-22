import IMP
import IMP.test
import IMP.core
import IMP.algebra
import pickle
try:
    import jax
except ImportError:
    jax = None


class JAXMover(IMP.core.MonteCarloMover):
    def __init__(self, m, state, ratio):
        super().__init__(m, "JAXMover%1%")
        self.state = state
        self.ratio = ratio

    def _get_jax(self):
        def init_func(key):
            return self.state
        def propose_func(jm, state):
            return jm, state + 1, self.ratio
        return self._wrap_jax(init_func, propose_func)


class Test(IMP.test.TestCase):

    def make_system(self):
        self.m = IMP.Model()
        self.ps = []
        self.mvs = []
        for i in range(10):
            p = IMP.Particle(self.m)
            self.ps.append(p)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0, 0, 0))
            IMP.core.XYZ(p).set_coordinates_are_optimized(True)
            self.mvs.append(IMP.core.BallMover(self.m, p, 1.))
            self.mvs[-1].set_was_used(True)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of SerialMover"""
        from IMP._jax_util import _get_jax_model
        m = IMP.Model()
        mvs = []
        for i in range(5):
            mvs.append(JAXMover(m, i * 100, 0.2 * i))
        mvr = IMP.core.SerialMover(mvs)
        ji = mvr._get_jax()
        init_func = jax.jit(ji.init_func)
        sms = init_func(jax.random.key(42))
        self.assertEqual(sms.imov, -1)
        # SerialMover initial state should contain initial state of each
        # JAXMover
        self.assertEqual(sms.mover_state, [0, 100, 200, 300, 400])

        X = _get_jax_model(m, [])
        propose_func = jax.jit(ji.propose_func)

        X, sms, ratio = propose_func(X, sms)
        self.assertEqual(sms.imov, 0)
        # First mover should have triggered
        self.assertEqual(sms.mover_state, [1, 100, 200, 300, 400])
        self.assertAlmostEqual(ratio, 0.0, delta=0.01)

        X, sms, ratio = propose_func(X, sms)
        self.assertEqual(sms.imov, 1)
        # Second mover should have triggered
        self.assertEqual(sms.mover_state, [1, 101, 200, 300, 400])
        self.assertAlmostEqual(ratio, 0.2, delta=0.01)

    def test_pickle(self):
        """Test (un-)pickle of SerialMover"""
        self.make_system()
        mvr = IMP.core.SerialMover(self.mvs)
        mvr.set_name("foo")
        dump = pickle.dumps(mvr)

        newmvr = pickle.loads(dump)
        self.assertEqual(newmvr.get_name(), "foo")
        self.assertEqual(len(newmvr.get_movers()), 10)


if __name__ == '__main__':
    IMP.test.main()
