import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom

vxkey = IMP.FloatKey('vx')
vykey = IMP.FloatKey('vy')
vzkey = IMP.FloatKey('vz')

class MolecularDynamicsStateTests(IMP.test.TestCase):
    """Test molecular dynamics optimizer states"""

    def setup_particles(self, coords):
        m = IMP.Model()
        ps = []
        for c in coords:
            p = IMP.Particle(m)
            x = IMP.core.XYZ.setup_particle(p, c[0])
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
        s.remove_rigid_motion()
        self.assertEqual(ps[0].get_value(vxkey), 0.)
        self.assertEqual(ps[0].get_value(vykey), 0.)
        self.assertEqual(ps[0].get_value(vzkey), 0.)

if __name__ == '__main__':
    IMP.test.main()
