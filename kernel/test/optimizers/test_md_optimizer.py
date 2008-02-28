import unittest
import IMP.utils
import IMP.test, IMP

xkey = IMP.FloatKey('x')
ykey = IMP.FloatKey('y')
zkey = IMP.FloatKey('z')
masskey = IMP.FloatKey('mass')
vxkey = IMP.FloatKey('vx')

# Conversion from derivatives (in kcal/mol/A) to acceleration (A/fs/fs)
kcal2mod = 4.1868e-4
# Mass of Carbon-12 (g/mol)
cmass = 12.011

class XTransRestraint(IMP.Restraint):
    """Attempt to move the whole system along the x axis"""
    def __init__(self, strength):
        IMP.Restraint.__init__(self)
        self.strength = strength

    def evaluate(self, accum):
        e = 0.
        for p in self.get_model().get_particles():
            e += p.get_value(xkey) * self.strength
        if accum:
            for p in self.get_model().get_particles():
                p.add_to_derivative(xkey, self.strength, accum)
                p.add_to_derivative(ykey, 0.0, accum)
                p.add_to_derivative(zkey, 0.0, accum)
        return e
    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")


class WriteTrajState(IMP.OptimizerState):
    """Write system coordinates (trajectory) into a Python list"""
    def __init__(self, traj):
        IMP.OptimizerState.__init__(self)
        self.traj = traj
    def update(self):
        model = self.get_optimizer().get_model()
        self.traj.append([(p.get_value(xkey), p.get_value(ykey),
                           p.get_value(zkey), p.get_value(vxkey)) \
                           for p in model.get_particles()])


class MolecularDynamicsTests(IMP.test.TestCase):
    """Test molecular dynamics optimizer"""

    def setUp(self):
        """Set up particles and optimizer"""

        self.model = IMP.Model()
        self.particles = []
        self.particles.append(IMP.utils.XYZParticle(self.model,
                                                    -43.0, 65.0, 93.0))
        self.particles[-1].add_attribute(masskey, cmass, False)
        self.md = IMP.MolecularDynamics()
        self.md.set_model(self.model)

    def _check_trajectory(self, coor, traj, timestep, vxfunc):
        """Check generated trajectory against that predicted using vxfunc"""
        vx = 0.
        msg = "Predicted coordinate %.5f doesn't match generated %.5f, " + \
              "for step %d, coordinate %d[%d]"
        velmsg = "Predicted velocity %.5f doesn't match generated %.5f, " + \
                 "for step %d, particle %d"
        for (num, step) in enumerate(traj):
            for n in range(len(coor)):
                coor[n][0] += vx * timestep
                self.assert_(abs(vx - step[n][3]) <= 1e-3,
                             velmsg % (vx, step[n][3], num, n))
                for d in range(3):
                    self.assert_(abs(coor[n][d] - step[n][d]) <= 1e-3,
                                 msg % (coor[n][d], step[n][d], num, n, d))
            vx = vxfunc(vx)

    def _optimize_model(self, timestep):
        """Run a short MD optimization on the model."""
        traj = []
        start = [[p.get_value(xkey), p.get_value(ykey), p.get_value(zkey)] \
                 for p in self.model.get_particles()]
        state = WriteTrajState(traj)
        self.md.add_optimizer_state(state)
        self.md.set_time_step(timestep)
        self.md.optimize(50)
        return start, traj

    def test_nonrigid_translation(self):
        """Check that non-rigid MD translation is Newtonian"""
        timestep = 4.0
        strength = 50.0
        r = XTransRestraint(strength)
        self.model.add_restraint(r)
        (start, traj) = self._optimize_model(timestep)
        delttm = -timestep * kcal2mod / cmass
        self._check_trajectory(start, traj, timestep,
                               lambda a: a + strength * delttm)

    def test_non_xyz(self):
        """Should skip particles without xyz attributes"""
        p = IMP.Particle()
        self.model.add_particle(p)
        p.add_attribute(IMP.FloatKey("attr"), 0.0, True)
        self.md.optimize(100)

    def test_make_velocities(self):
        """Test that MD generates particle velocities"""
        self.md.optimize(0)
        keys = [IMP.FloatKey(x) for x in ("vx", "vy", "vz")]
        for p in self.model.get_particles():
            for key in keys:
                self.assert_(p.has_attribute(key))

    def test_temperature(self):
        """Check temperature"""
        # Averages for temperature only make sense if we have a comparatively
        # large number of particles:
        for i in range(100):
            self.particles.append(IMP.utils.XYZParticle(self.model,
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
        tkinetic = self.md.get_kinetic_temperature(ekinetic)
        self.assertNotEqual(ekinetic, 0.0)
        diff = abs(tkinetic - 100.0)
        self.assert_(diff < 10.0)

if __name__ == '__main__':
    unittest.main()
