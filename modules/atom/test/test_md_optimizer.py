import IMP
import IMP.test
import IMP.core
import IMP.atom
import functools
try:
    import jax
    import jax.random
except ImportError:
    jax = None

xkey = IMP.FloatKey('x')
ykey = IMP.FloatKey('y')
zkey = IMP.FloatKey('z')
masskey = IMP.FloatKey('mass')
linvelkey = IMP.atom.LinearVelocity.get_velocity_key()

# Conversion from derivatives (in kcal/mol/A) to acceleration (A/fs/fs)
kcal2mod = 4.1868e-4
# Mass of Carbon-12 (g/mol)
cmass = 12.011


class XTransRestraint(IMP.Restraint):

    """Attempt to move the whole system along the x axis"""

    def __init__(self, m, strength):
        super().__init__(m, "XTransRestraint %1%")
        self.strength = strength
        self.set_log_level(IMP.SILENT)

    def unprotected_evaluate(self, accum):
        m = self.get_model()
        e = 0.
        for p in m.get_particle_indexes():
            e += m.get_attribute(xkey, p) * self.strength
        if accum:
            for pi in m.get_particle_indexes():
                m.add_to_derivative(xkey, pi, self.strength, accum)
        return e

    def _get_jax(self):
        import jax.numpy as jnp
        indexes = self.get_model().get_particle_indexes()
        strength = self.strength
        def jax_restraint(X):
            xyzs = X['xyz'][indexes]
            return jnp.sum(xyzs[:,0] * strength)
        return self._wrap_jax(jax_restraint)

    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")

    def do_show(self, fh):
        fh.write("Test restraint")

    def do_get_inputs(self):
        m = self.get_model()
        return IMP.get_particles(m, m.get_particle_indexes())


class WriteTrajState(IMP.OptimizerState):

    """Write system coordinates (trajectory) into a Python list"""

    def __init__(self, m, traj):
        super().__init__(m, "WriteTrajState%1%")
        self.traj = traj

    def _get_jax(self):
        # No explicit JAX implementation; JAX data will be synced back to
        # the IMP.Model before calling do_update()
        return None

    def do_update(self, call):
        model = self.get_optimizer().get_model()
        self.traj.append([(model.get_attribute(xkey, p),
                           model.get_attribute(ykey, p),
                           model.get_attribute(zkey, p),
                           IMP.atom.LinearVelocity(model, p).get_velocity())
                          for p in model.get_particle_indexes()])


class JAXOptimizerState(IMP.OptimizerState):
    def __init__(self, m, name):
        super().__init__(m, name)

    def _get_jax(self):
        import IMP._jax_util
        name = self.get_name()

        def init_func(ms):
            ms.optimizer_states[name] = {'calls': 0}
            return ms

        def apply_func(ms):
            ms.optimizer_states[name]['calls'] += 1
            return ms

        return self._wrap_jax(init_func, apply_func)


class Tests(IMP.test.TestCase):

    """Test molecular dynamics optimizer"""

    def make_model(self):
        """Set up particles and optimizer"""
        self.model = IMP.Model()
        self.particles = []
        self.particles.append(self.create_point_particle(self.model,
                                                         -43.0, 65.0, 93.0))
        self.particles[-1].add_attribute(masskey, cmass, False)
        self.md = IMP.atom.MolecularDynamics(self.model)

    def test_standard_object_methods(self):
        """Check MD standard object methods"""
        self.make_model()
        self.check_standard_object_methods(self.md)

    def test_get_scoring_function(self):
        """Test get_scoring_function()"""
        self.make_model()
        r = XTransRestraint(self.model, 1.0)
        sf = IMP.core.RestraintsScoringFunction([r])
        self.md.set_scoring_function(sf)

        new_sf = self.md.get_scoring_function()
        self.assertIs(type(new_sf), IMP.ScoringFunction)
        self.assertFalse(hasattr(new_sf, 'restraints'))
        # We should be able to get the original RestraintsScoringFunction
        dsf = new_sf.get_derived_object()
        self.assertIsInstance(dsf, IMP.core.RestraintsScoringFunction)
        self.assertEqual(len(dsf.restraints), 1)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation"""
        def make_md():
            timestep = 4.0
            strength = 50.0
            self.make_model()
            r = XTransRestraint(self.model, strength)
            sf = IMP.core.RestraintsScoringFunction([r])
            self.md.set_scoring_function(sf)
            self.md.set_maximum_time_step(timestep)

        self._compare_jax_implementation(make_md)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_velocity_cap(self):
        """Test JAX implementation with velocity cap"""
        def make_md():
            timestep = 4.0
            strength = 5000.0
            self.make_model()
            r = XTransRestraint(self.model, strength)
            sf = IMP.core.RestraintsScoringFunction([r])
            self.md.set_scoring_function(sf)
            self.md.set_maximum_time_step(timestep)
            self.md.set_velocity_cap(0.3)

        self._compare_jax_implementation(make_md)

    def _compare_jax_implementation(self, make_md):
        # Run with original C++ code
        make_md()
        self.md.optimize(50)
        final_cpp = IMP.core.XYZ(self.particles[0]).get_coordinates()
        final_ke = self.md.get_kinetic_energy()
        final_temp = self.md.get_kinetic_temperature(final_ke)

        # Run with JAX code, low level
        make_md()
        ji = self.md._get_jax()
        X = ji.get_model_state()

        jit_init_func = jax.jit(ji.init_func)
        md_state = jit_init_func(X, key=jax.random.key(42))

        def run_opt(X, apply_func, nsteps):
            return jax.lax.fori_loop(0, nsteps, apply_func, X)

        jit_apply_func = jax.jit(functools.partial(
            run_opt, apply_func=lambda i, x: ji.apply_func(x), nsteps=50))
        md_state = jit_apply_func(md_state)
        self.assertEqual(md_state.steps, 50)
        self.assertAlmostEqual(md_state.time_step,
                               self.md.get_maximum_time_step(), delta=0.1)
        # Final coordinates should match those from C++
        self.assertLess((final_cpp - md_state.X['xyz'][0]).get_magnitude(),
                        1e-3)
        # Final kinetic energy and temperature should match that from C++
        ke = md_state.get_kinetic_energy()
        self.assertAlmostEqual(ke, final_ke, delta=0.1)
        self.assertAlmostEqual(md_state.get_kinetic_temperature(ke),
                               final_temp, delta=2.0)

        # Run with JAX code, high level
        make_md()
        self.md._optimize_jax(50)
        final_jax = IMP.core.XYZ(self.particles[0]).get_coordinates()
        self.assertLess((final_cpp - final_jax).get_magnitude(), 1e-3)

    def _check_trajectory(self, coor, traj, timestep, vxfunc):
        """Check generated trajectory against that predicted using vxfunc"""
        vx = 0.
        msg = "Predicted coordinate %.5f doesn't match generated %.5f, " + \
              "for step %d, coordinate %d[%d]"
        velmsg = "Predicted velocity %.5f doesn't match generated %.5f, " + \
                 "for step %d, particle %d"
        for (num, step) in enumerate(traj[:-1]):
            newvx = vxfunc(vx)
            print(num)
            for n in range(len(coor)):
                self.assertAlmostEqual(vx, step[n][3][0], delta=1e-3,
                                       msg=velmsg % (vx, step[n][3][0],
                                                     num, n))
                for d in range(3):
                    self.assertAlmostEqual(coor[n][d], step[n][d], delta=1e-3,
                                           msg=msg % (coor[n][d], step[n][d],
                                                      num, n, d))
                coor[n][0] += (newvx + vx) / 2.0 * timestep
            vx = newvx

    def _optimize_model(self, timestep, restraints, use_jax=False):
        """Run a short MD optimization on the model."""
        start = [[self.model.get_attribute(xkey, p),
                  self.model.get_attribute(ykey, p),
                  self.model.get_attribute(zkey, p)]
                 for p in self.model.get_particle_indexes()]
        # Add starting (step 0) position to the trajectory, with zero velocity
        traj = [[(x[0], x[1], x[2], [0,0,0]) for x in start]]
        state = WriteTrajState(self.model, traj)
        sf = IMP.core.RestraintsScoringFunction(restraints)
        self.md.set_scoring_function(sf)
        self.md.add_optimizer_state(state)
        self.md.set_maximum_time_step(timestep)
        if use_jax:
            self.md._optimize_jax(50)
        else:
            self.md.optimize(50)
        return start, traj

    def test_nonrigid_translation(self):
        """Check that non-rigid MD translation is Newtonian"""
        # If we have JAX support, also test the JAX implementation
        if jax is None:
            use_jax_opts = [False]
        else:
            use_jax_opts = [False, True]
        for use_jax in use_jax_opts:
            self.make_model()
            timestep = 4.0
            strength = 50.0
            r = XTransRestraint(self.model, strength)
            (start, traj) = self._optimize_model(timestep, [r],
                                                 use_jax=use_jax)
            delttm = -timestep * kcal2mod / cmass
            self._check_trajectory(start, traj, timestep,
                                   lambda a: a + strength * delttm)

    def test_velocity_cap(self):
        """Check that velocity capping works"""
        self.make_model()
        timestep = 4.0
        strength = 5000.0
        r = XTransRestraint(self.model, strength)
        self.md.set_velocity_cap(0.3)
        (start, traj) = self._optimize_model(timestep, [r])
        # Strength is so high that velocity should max out at the cap
        for i in range(49):
            oldx = traj[i][0][0]
            newx = traj[i + 1][0][0]
            # Calculate velocity from change in position
            self.assertAlmostEqual((oldx - newx) / timestep, 0.3, delta=1e-5)

    def test_non_xyz(self):
        """Should skip particles without xyz attributes"""
        self.make_model()
        p = IMP.Particle(self.model)
        p.add_attribute(IMP.FloatKey("attr"), 0.0, True)
        r = IMP.RestraintSet(self.model)
        self.md.set_scoring_function(r)
        self.md.optimize(100)

    def test_make_velocities(self):
        """Test that MD generates particle velocities"""
        self.make_model()
        r = IMP.RestraintSet(self.model)
        self.md.set_scoring_function(r)
        self.md.optimize(0)
        for p in self.model.get_particle_indexes():
            self.assertTrue(self.model.get_has_attribute(linvelkey, p))

    def _check_temperature(self, desired, tolerance):
        """Check the temperature of the system"""
        ekinetic = self.md.get_kinetic_energy()
        tkinetic = self.md.get_kinetic_temperature(ekinetic)
        self.assertAlmostEqual(tkinetic, desired,
                               msg="Temperature %f does not match expected %f within %f"
                               % (tkinetic, desired, tolerance),
                               delta=tolerance)

    def test_temperature(self):
        """Check temperature"""
        self.make_model()
        # Averages for temperature only make sense if we have a comparatively
        # large number of particles:
        for i in range(500):
            self.particles.append(self.create_point_particle(self.model,
                                                             -43.0, 65.0, 93.0))
            self.particles[-1].add_attribute(masskey, cmass, False)
        # Initial temperature should be zero:
        ekinetic = self.md.get_kinetic_energy()
        tkinetic = self.md.get_kinetic_temperature(ekinetic)
        self.assertEqual(ekinetic, 0.0)
        self.assertEqual(tkinetic, 0.0)
        # After assigning T=100K, kinetic temp should be 100K:
        self.md.assign_velocities(100.0)
        ekinetic = self.md.get_kinetic_energy()
        self.assertNotEqual(ekinetic, 0.0)
        self._check_temperature(100.0, 20.0)
        # Make sure that the random number generator is working properly;
        # we should get different particle velocities each time we assign
        # velocities (NOT the case with r452 or earlier):
        velocity = IMP.atom.LinearVelocity(self.particles[0]).get_velocity()[0]
        self.md.assign_velocities(100.0)
        velocity2 = IMP.atom.LinearVelocity(self.particles[0]).get_velocity()[0]
        self.assertNotAlmostEqual(velocity, velocity2, delta=1e-6)
        # Kinetic energy, however, should be almost identical
        ekinetic2 = self.md.get_kinetic_energy()
        self.assertAlmostEqual(ekinetic, ekinetic2, delta=1e-2)

    def test_get_optimizer_states(self):
        """Test get_optimizer_states() method"""
        self.make_model()
        wrtraj = WriteTrajState(self.model, [])
        scaler = IMP.atom.VelocityScalingOptimizerState(self.model,
                                                        self.particles, 298.0)
        scaler.set_period(10)
        self.md.add_optimizer_state(wrtraj)
        self.md.add_optimizer_state(scaler)
        m = self.md.get_optimizer_states()
        self.assertEqual(len(m), 2)
        for a in m:
            self.assertIsInstance(a, IMP.OptimizerState)

    def test_rescaling(self):
        """Test thermostatting by velocity rescaling"""
        self.make_model()
        for i in range(100):
            self.particles.append(self.create_point_particle(self.model,
                                                             -43.0, 65.0, 93.0))
            self.particles[-1].add_attribute(masskey, cmass, False)
        self.md.assign_velocities(100.0)
        scaler = IMP.atom.VelocityScalingOptimizerState(self.model,
                                                        self.particles, 298.0)
        scaler.set_period(10)
        self.md.add_optimizer_state(scaler)
        r = IMP.RestraintSet(self.model)
        self.md.set_scoring_function(r)
        self.md.optimize(10)
        # Temperature should have been rescaled to 298.0 at some point:
        self._check_temperature(298.0, 0.1)
        # Also check immediate rescaling:
        scaler.set_temperature(50.0)
        scaler.rescale_velocities()
        self._check_temperature(50.0, 0.1)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_rescaling(self):
        """Test JAX thermostatting by velocity rescaling"""
        self.make_model()
        # With strength 0 this is a "do-nothing" restraint
        r = XTransRestraint(self.model, 0.0)
        sf = IMP.core.RestraintsScoringFunction([r])
        self.md.set_scoring_function(sf)
        self.md.assign_velocities(100.0)
        scaler = IMP.atom.VelocityScalingOptimizerState(self.model,
                                                        self.particles, 298.0)
        self.md.add_optimizer_state(scaler)
        self.md._optimize_jax(10)
        self._check_temperature(298.0, 0.1)

    def test_linear_velocity_numpy(self):
        """Test access to linear velocities from NumPy"""
        self.make_model()
        m1 = IMP.Model()
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)
        v1 = IMP.atom.LinearVelocity.setup_particle(
            p1, IMP.algebra.Vector3D(0, 1, 2))
        v2 = IMP.atom.LinearVelocity.setup_particle(
            p2, IMP.algebra.Vector3D(3, 4, 5))
        if IMP.IMP_KERNEL_HAS_NUMPY:
            vs = m1.get_vector3ds_numpy(
                IMP.atom.LinearVelocity.get_velocity_key())
            self.assertEqual(vs.shape, (2, 3))
            self.assertAlmostEqual(vs[0][2], 2.0, delta=1e-5)
            self.assertAlmostEqual(vs[1][2], 5.0, delta=1e-5)
            vs[0][0] = 42.0
            self.assertAlmostEqual(vs[0][0], 42.0, delta=1e-5)

            # Read-only array should raise ValueError on assignment
            vs = m1.get_vector3ds_numpy(
                IMP.atom.LinearVelocity.get_velocity_key(), read_only=True)
            self.assertRaises(ValueError, vs[0].__setitem__, 0, 42.0)
        else:
            self.assertRaises(NotImplementedError, m1.get_vector3ds_numpy)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_optimizer_state(self):
        """Test pure JAX OptimizerState"""
        def make_md():
            timestep = 4.0
            strength = 50.0
            self.make_model()
            # Make sure JAX MD works with more than one particle
            self.particles.append(self.create_point_particle(self.model,
                                                             -43.0, 65.0, 93.0))
            self.particles[-1].add_attribute(masskey, cmass, False)
            r = XTransRestraint(self.model, strength)
            sf = IMP.core.RestraintsScoringFunction([r])
            self.md.set_scoring_function(sf)
            self.md.set_maximum_time_step(timestep)
            state1 = JAXOptimizerState(self.model, name="State1")
            self.md.add_optimizer_state(state1)
            state2 = JAXOptimizerState(self.model, name="State2")
            state2.set_period(2)
            self.md.add_optimizer_state(state2)

        # Low level
        make_md()
        ji = self.md._get_jax()
        X = ji.get_model_state()
        jit_init_func = jax.jit(ji.init_func)
        md_state = jit_init_func(X, key=jax.random.key(42))

        jit_apply_func = jax.jit(ji.apply_func)
        md_state = jit_apply_func(md_state)
        md_state = jit_apply_func(md_state)
        self.assertEqual(md_state.optimizer_states['State1']['calls'], 2)
        self.assertEqual(md_state.optimizer_states['State2']['calls'], 1)

        # High level
        make_md()
        self.md._optimize_jax(2)


if __name__ == '__main__':
    IMP.test.main()
