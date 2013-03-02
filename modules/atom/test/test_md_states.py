import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom

vxkey = IMP.FloatKey('vx')
vykey = IMP.FloatKey('vy')
vzkey = IMP.FloatKey('vz')

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
                p.add_attribute(vxkey, c[1][0])
                p.add_attribute(vykey, c[1][1])
                p.add_attribute(vzkey, c[1][2])
                ps.append(p)
        return m, ps

    def test_remove_rigid_translation(self):
        """Ensure that rigid translation is removed"""
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0,0,0),
                                       IMP.algebra.Vector3D(10,0,0)],
                                      [IMP.algebra.Vector3D(10,0,0),
                                       IMP.algebra.Vector3D(-20,0,0)]])
        s = IMP.atom.RemoveRigidMotionOptimizerState(ps, 1)
        s.remove_rigid_motion()
        self.assertEqual(ps[0].get_value(vxkey), 15.)
        self.assertEqual(ps[1].get_value(vxkey), -15.)
        for p in ps:
            self.assertEqual(p.get_value(vykey), 0.)
            self.assertEqual(p.get_value(vzkey), 0.)

    def test_remove_rigid_rotation(self):
        """Ensure that rigid rotation is removed"""
        # Create 4 points at the vertices of a tetrahedron centered at origin
        xs = [IMP.algebra.Vector3D(x) for x in [(-10,-10,-10),(10,10,10),
                                                (10,-10,-10),(-10,10,10)]]
        # Add velocities that would spin it about an axis through the origin
        # that it not aligned with the x,y,or z axes
        torque = IMP.algebra.Vector3D(5,8,10)
        vs = [IMP.algebra.get_vector_product(x,torque) for x in xs]
        m, ps = self.setup_particles(zip(xs, vs))
        s = IMP.atom.RemoveRigidMotionOptimizerState(ps, 1)
        s.remove_rigid_motion()
        # We started with no net linear momentum, so removing the angular
        # momentum should cause the system to become stationary
        for p in ps:
            self.assertAlmostEqual(p.get_value(vxkey), 0., delta=1e-6)
            self.assertAlmostEqual(p.get_value(vykey), 0., delta=1e-6)
            self.assertAlmostEqual(p.get_value(vzkey), 0., delta=1e-6)

    def test_remove_rigid_one_particle(self):
        """Ensure that rigid removal works with a 1-particle system"""
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0,0,0),
                                       IMP.algebra.Vector3D(10,0,0)]])
        s = IMP.atom.RemoveRigidMotionOptimizerState(ps, 1)
        self.assertEqual(s.get_skip_steps(), 1)
        s.remove_rigid_motion()
        self.assertEqual(ps[0].get_value(vxkey), 0.)
        self.assertEqual(ps[0].get_value(vykey), 0.)
        self.assertEqual(ps[0].get_value(vzkey), 0.)

    def test_berendsen_thermostat(self):
        """Test Berendsen thermostat"""
        # With a shorter coupling time, fewer steps should be needed
        # to reach the set temperature
        for (coupling, steps) in [(8.0, 16), (6.0, 10)]:
            m, ps = self.setup_particles([[IMP.algebra.Vector3D(0,0,0),
                                           IMP.algebra.Vector3D(0.1,0,0)]])
            scaler = IMP.atom.BerendsenThermostatOptimizerState(
                                              ps, 298.0, coupling)
            md = IMP.atom.MolecularDynamics(m)
            md.set_maximum_time_step(4.0)
            md.add_optimizer_state(scaler)
            md.optimize(0)
            ts = []
            for i in range(20):
                ts.append(md.get_kinetic_temperature(md.get_kinetic_energy()))
                scaler.rescale_velocities()
            # Temperature should decrease from start to set temp
            self.assertAlmostEqual(ts[0], 4009.0, delta=0.2)
            self.assertGreater(ts[steps-1], 298.1)
            # Make sure that once set temperature is reached, it is maintained
            for i in range(steps, 20):
                self.assertAlmostEqual(ts[i], 298.0, delta=0.1)

    def test_langevin_thermostat(self):
        """Test Langevin thermostat"""
        # Need many particles due to random forces
        m, ps = self.setup_particles([[IMP.algebra.Vector3D(0,0,0),
                                       IMP.algebra.Vector3D(0.1,0,0)]],
                                     copies=50)
        scaler = IMP.atom.LangevinThermostatOptimizerState(
                                              ps, 298.0, 0.1)
        md = IMP.atom.MolecularDynamics(m)
        md.set_maximum_time_step(4.0)
        md.add_optimizer_state(scaler)
        md.optimize(0)
        ts = []
        for i in range(140):
            ts.append(md.get_kinetic_temperature(md.get_kinetic_energy()))
            scaler.rescale_velocities()
        # After a while, temperature should have stabilized at set value
        equilibrium_temp = sum(ts[40:140])/100.0
        self.assertAlmostEqual(equilibrium_temp, 298.0, delta=20.0)

if __name__ == '__main__':
    IMP.test.main()
