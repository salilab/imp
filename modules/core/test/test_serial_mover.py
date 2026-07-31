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
    def __init__(self, m, state, ratio, key):
        super().__init__(m, "JAXMover%1%")
        self.state = state
        self.ratio = ratio
        self._key = key

    def _get_jax(self):
        def init_func(key):
            return self.state

        def propose_func(jm, state):
            return jm, state + 1, self.ratio
        return self._wrap_jax(init_func, propose_func, keys=[self._key])


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
        fk1 = IMP.FloatKey("jax float key 1")
        fk2 = IMP.FloatKey("jax float key 2")
        for i in range(5):
            mvs.append(JAXMover(m, i * 100, 0.2 * i, [fk1, fk2][i % 2]))
        mvr = IMP.core.SerialMover(mvs)
        ji = mvr._get_jax()
        self.assertEqual(ji._keys, frozenset([fk1, fk2]))
        init_func = jax.jit(ji.init_func)
        sms = init_func(jax.random.key(42))
        self.assertEqual(sms.imov, -1)
        # SerialMover initial state should contain initial state of each
        # JAXMover
        self.assertEqual(sms.mover_state, [0, 100, 200, 300, 400])

        X = _get_jax_model(m, [])
        propose_func = jax.jit(ji.propose_func)
        accept_func = jax.jit(ji.accept_func)

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

        # Make sure mover wraps back to zero
        for i in range(7):
            X, sms, ratio = propose_func(X, sms)
        sms = accept_func(sms)
        self.assertEqual(sms.imov, 3)
        self.assertEqual(sms.mover_state, [2, 102, 202, 302, 401])
        self.assertEqual(list(sms.proposed_mover_steps), [2, 2, 2, 2, 1])
        self.assertEqual(list(sms.accepted_mover_steps), [0, 0, 0, 1, 0])
        self.assertAlmostEqual(ratio, 0.6, delta=0.01)

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
