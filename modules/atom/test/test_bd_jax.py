import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra
try:
    import jax
except ImportError:
    jax = None


class XTransRestraint(IMP.Restraint):
    """Apply a constant force to all particles along the x axis"""

    def __init__(self, m, strength):
        super().__init__(m, "XTransRestraint %1%")
        self.strength = strength

    def unprotected_evaluate(self, accum):
        m = self.get_model()
        xkey = IMP.FloatKey('x')
        e = 0.
        for p in m.get_particle_indexes():
            e += m.get_attribute(xkey, p) * self.strength
            if accum:
                m.add_to_derivative(xkey, p, self.strength, accum)
        return e

    def _get_jax(self, space):
        import jax.numpy as jnp
        indexes = self.get_model().get_particle_indexes()
        strength = self.strength

        def jax_restraint(jm):
            return jnp.sum(jm['xyz'][indexes, 0] * strength)
        return self._wrap_jax(jax_restraint)

    def do_get_inputs(self):
        m = self.get_model()
        return IMP.get_particles(m, m.get_particle_indexes())


class Tests(IMP.test.TestCase):

    def make_model(self, n, diffusion_coefficient):
        m = IMP.Model()
        ps = []
        for i in range(n):
            d = IMP.atom.Diffusion.setup_particle(
                IMP.Particle(m), IMP.algebra.Vector3D(0, 0, 0),
                diffusion_coefficient)
            d.set_coordinates_are_optimized(True)
            ps.append(d)
        bd = IMP.atom.BrownianDynamics(m)
        bd.set_maximum_time_step(10.0)
        bd.set_temperature(300.)
        return m, bd, ps

    def run_bd(self, bd, nsteps, use_jax):
        if use_jax:
            return bd._optimize_jax(nsteps)
        else:
            return bd.optimize(nsteps)

    def get_mean_coordinates(self, ps):
        c = IMP.algebra.Vector3D(0, 0, 0)
        for p in ps:
            c += p.get_coordinates()
        return c / len(ps)

    def test_getters(self):
        """Test BrownianDynamics getters (used by JAX)"""
        m, bd, ps = self.make_model(1, 1.0)
        self.assertGreater(bd.get_maximum_move(), 1e20)
        self.assertFalse(bd.get_use_stochastic_runge_kutta())
        bd.set_maximum_move(4.0)
        bd.set_use_stochastic_runge_kutta(True)
        self.assertAlmostEqual(bd.get_maximum_move(), 4.0, delta=1e-6)
        self.assertTrue(bd.get_use_stochastic_runge_kutta())

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_diffusion(self):
        """Test JAX BD free diffusion matches C++ and theory"""
        n, D, nsteps = 1000, 1e-3, 20
        for use_jax in (False, True):
            m, bd, ps = self.make_model(n, D)
            # Note that C++ BD does not work with an empty scoring function
            # (derivatives are not initialized) so use a zero restraint
            bd.set_scoring_function([XTransRestraint(m, 0.)])
            self.run_bd(bd, nsteps, use_jax)
            t = bd.get_current_time()
            self.assertAlmostEqual(t, nsteps * 10.0, delta=1e-6)
            msd = sum(p.get_coordinates().get_squared_magnitude()
                      for p in ps) / n
            # MSD should be 6Dt; with 1000 particles the standard error
            # is ~2.6%
            self.assertAlmostEqual(msd / (6. * D * t), 1.0, delta=0.12)
            # No drift
            mean = self.get_mean_coordinates(ps)
            self.assertLess(mean.get_magnitude(),
                            5. * (2. * D * t / n) ** 0.5 * 3 ** 0.5)

    def _check_drift(self, srk, max_move=None):
        n, D, nsteps, strength = 200, 1e-3, 10, -1.0
        results = []
        for use_jax in (False, True):
            m, bd, ps = self.make_model(n, D)
            bd.set_use_stochastic_runge_kutta(srk)
            if max_move is not None:
                bd.set_maximum_move(max_move)
            r = XTransRestraint(m, strength)
            bd.set_scoring_function([r])
            self.run_bd(bd, nsteps, use_jax)
            results.append(self.get_mean_coordinates(ps))
        return results

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_drift(self):
        """Test JAX BD force drift matches C++"""
        D, dt, nsteps = 1e-3, 10.0, 10
        cpp, jx = self._check_drift(srk=False)
        expected = -(-1.0) * D * dt * nsteps / IMP.atom.get_kt(300.)
        # noise of the mean over 200 particles is ~0.03A
        for c in (cpp, jx):
            self.assertAlmostEqual(c[0], expected, delta=0.15)
            self.assertAlmostEqual(c[1], 0., delta=0.15)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_drift_srk(self):
        """Test JAX BD with stochastic Runge-Kutta matches C++"""
        cpp, jx = self._check_drift(srk=True)
        # With a constant force, the SRK correction is zero
        self.assertLess((cpp - jx).get_magnitude(), 0.2)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_maximum_move(self):
        """Test JAX BD maximum move matches C++"""
        n, D, nsteps = 50, 1.0, 5
        for use_jax in (False, True):
            m, bd, ps = self.make_model(n, D)
            bd.set_maximum_move(0.1)
            r = XTransRestraint(m, -1000.0)
            bd.set_scoring_function([r])
            self.run_bd(bd, nsteps, use_jax)
            # Force term dominates, so every particle should move the
            # maximum distance in x each step
            for p in ps:
                c = p.get_coordinates()
                self.assertAlmostEqual(c[0], 0.1 * nsteps, delta=1e-5)
                for i in (1, 2):
                    self.assertLessEqual(abs(c[i]), 0.1 * nsteps + 1e-5)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_rigid_body_unsupported(self):
        """Test JAX BD rejects rigid bodies"""
        m = IMP.Model()
        bd = IMP.atom.BrownianDynamics(m)
        members = [IMP.core.XYZR.setup_particle(
                       IMP.Particle(m), IMP.algebra.Sphere3D(
                           IMP.algebra.Vector3D(x, 0, 0), 1.0))
                   for x in (0., 2.)]
        rb = IMP.core.RigidBody.setup_particle(IMP.Particle(m), members)
        rb.set_coordinates_are_optimized(True)
        IMP.atom.RigidBodyDiffusion.setup_particle(rb)
        bd.set_scoring_function([])
        self.assertRaises(NotImplementedError, bd._get_jax)


if __name__ == '__main__':
    IMP.test.main()
