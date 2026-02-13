import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
try:
    import jax
except ImportError:
    jax = None


class DoNothingRestraint(IMP.Restraint):
    def __init__(self, m):
        super().__init__(m, "DoNothingRestraint %1%")
    def do_get_inputs(self):
        return []
    def _get_jax(self):
        return self._wrap_jax(lambda jm: 0.0)


class Tests(IMP.test.TestCase):

    """Test molecular dynamics optimizer states"""

    def setup_particles(self, coords, copies=1):
        m = IMP.Model()
        ps = []
        for i in range(copies):
            for c in coords:
                p = IMP.Particle(m)
                x = IMP.core.XYZ.setup_particle(p, c[0])
                x.set_coordinates_are_optimized(True)
                IMP.atom.Mass.setup_particle(p, 1.0)
                IMP.atom.LinearVelocity.setup_particle(p, c[1])
                ps.append(p)
        return m, ps

    def test_remove_rigid_translation(self):
        """Ensure that rigid translation is removed"""
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 0, 0)],
                                      [IMP.algebra.Vector3D(10, 0, 0),
                                       IMP.algebra.Vector3D(-20, 0, 0)]])
        s = IMP.atom.RemoveRigidMotionOptimizerState(m, ps)
        s.set_period(1)
        s.remove_rigid_motion()
        self.assertAlmostEqual(
                IMP.atom.LinearVelocity(ps[0]).get_velocity()[0], 15.,
                delta=1e-5)
        self.assertAlmostEqual(
                IMP.atom.LinearVelocity(ps[1]).get_velocity()[0], -15.,
                delta=1e-5)
        for p in ps:
            vel = IMP.atom.LinearVelocity(p).get_velocity()
            self.assertEqual(vel[1], 0.)
            self.assertEqual(vel[2], 0.)

    def test_remove_rigid_rotation(self):
        """Ensure that rigid rotation is removed"""
        # Create 4 points at the vertices of a tetrahedron centered at origin
        xs = [IMP.algebra.Vector3D(x) for x in [(-10, -10, -10), (10, 10, 10),
                                                (10, -10, -10), (-10, 10, 10)]]
        # Add velocities that would spin it about an axis through the origin
        # that it not aligned with the x,y,or z axes
        torque = IMP.algebra.Vector3D(5, 8, 10)
        vs = [IMP.algebra.get_vector_product(x, torque) for x in xs]
        m, ps = self.setup_particles(zip(xs, vs))
        s = IMP.atom.RemoveRigidMotionOptimizerState(m, ps)
        s.set_period(1)
        s.remove_rigid_motion()
        # We started with no net linear momentum, so removing the angular
        # momentum should cause the system to become stationary
        for p in ps:
            vel = IMP.atom.LinearVelocity(p).get_velocity()
            self.assertAlmostEqual(vel[0], 0., delta=1e-6)
            self.assertAlmostEqual(vel[1], 0., delta=1e-6)
            self.assertAlmostEqual(vel[2], 0., delta=1e-6)

    def test_remove_rigid_one_particle(self):
        """Ensure that rigid removal works with a 1-particle system"""
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(10, 0, 0)]])
        s = IMP.atom.RemoveRigidMotionOptimizerState(m, ps)
        s.set_period(1)
        self.assertEqual(s.get_period(), 1)
        s.remove_rigid_motion()
        vel = IMP.atom.LinearVelocity(ps[0]).get_velocity()
        self.assertEqual(vel[0], 0.)
        self.assertEqual(vel[1], 0.)
        self.assertEqual(vel[2], 0.)

    def test_berendsen_thermostat(self):
        """Test Berendsen thermostat"""
        # With a shorter coupling time, fewer steps should be needed
        # to reach the set temperature
        for (coupling, steps) in [(8.0, 16), (6.0, 10)]:
            m, ps = self.setup_particles([[IMP.algebra.Vector3D(0, 0, 0),
                                           IMP.algebra.Vector3D(0.1, 0, 0)]])
            scaler = IMP.atom.BerendsenThermostatOptimizerState(
                ps, 298.0, coupling)
            md = IMP.atom.MolecularDynamics(m)
            md.set_maximum_time_step(4.0)
            md.set_scoring_function([])
            md.optimize(0)  # ick
            md.add_optimizer_state(scaler)
            ts = []
            for i in range(20):
                ts.append(md.get_kinetic_temperature(md.get_kinetic_energy()))
                scaler.rescale_velocities()
            # Temperature should decrease from start to set temp
            print(ts)
            self.assertAlmostEqual(ts[0], 4009.0, delta=0.2)
            self.assertGreater(ts[steps - 1], 298.1)
            # Make sure that once set temperature is reached, it is maintained
            for i in range(steps, 20):
                self.assertAlmostEqual(ts[i], 298.0, delta=0.1)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_berendsen_thermostat(self):
        """Test JAX implementation of Berendsen thermostat"""
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(0.1, 0, 0)]],
                                     copies=10)
        scaler = IMP.atom.BerendsenThermostatOptimizerState(ps, 298.0, 8.0)
        md = IMP.atom.MolecularDynamics(m)
        md.set_maximum_time_step(4.0)
        r = DoNothingRestraint(m)
        sf = IMP.core.RestraintsScoringFunction([r])
        md.set_scoring_function(sf)
        md.add_optimizer_state(scaler)
        md._optimize_jax(20)
        ts = md.get_kinetic_temperature(md.get_kinetic_energy())
        self.assertAlmostEqual(ts, 298.0, delta=0.1)

    def test_langevin_thermostat(self):
        """Test Langevin thermostat"""
        # Need many particles due to random forces
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(0.1, 0, 0)]],
                                     copies=50)
        scaler = IMP.atom.LangevinThermostatOptimizerState(
            m, ps, 298.0, 0.1)
        md = IMP.atom.MolecularDynamics(m)
        md.set_maximum_time_step(4.0)
        md.add_optimizer_state(scaler)
        md.set_scoring_function([])
        md.optimize(0)
        ts = []
        for i in range(140):
            ts.append(md.get_kinetic_temperature(md.get_kinetic_energy()))
            scaler.rescale_velocities()
        # After a while, temperature should have stabilized at set value
        equilibrium_temp = sum(ts[40:140]) / 100.0
        self.assertAlmostEqual(equilibrium_temp, 298.0, delta=20.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_langevin_thermostat(self):
        """Test JAX implementation of Langevin thermostat"""
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(0.1, 0, 0)]],
                                     copies=1000)
        scaler = IMP.atom.LangevinThermostatOptimizerState(m, ps, 298.0, 0.1)
        md = IMP.atom.MolecularDynamics(m)
        md.set_maximum_time_step(4.0)
        r = DoNothingRestraint(m)
        sf = IMP.core.RestraintsScoringFunction([r])
        md.set_scoring_function(sf)
        md.add_optimizer_state(scaler)
        md._optimize_jax(100)
        ts = md.get_kinetic_temperature(md.get_kinetic_energy())
        self.assertAlmostEqual(ts, 298.0, delta=50.0)


if __name__ == '__main__':
    IMP.test.main()
