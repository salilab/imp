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
            x = IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(c[:3]))
            IMP.atom.Mass.setup_particle(p, 1.0)
            p.add_attribute(vxkey, c[3])
            p.add_attribute(vykey, c[4])
            p.add_attribute(vzkey, c[5])
            ps.append(p)
        return m, ps

    def test_remove_rigid_translation(self):
        """Ensure that rigid translation is removed"""
        m, ps = self.setup_particles([[0,0,0,10,0,0],
                                     [10,0,0,-20,0,0]])
        s = IMP.atom.RemoveRigidMotionOptimizerState(ps, 1)
        s.remove_rigid_motion()
        self.assertEqual(ps[0].get_value(vxkey), 15.)
        self.assertEqual(ps[1].get_value(vxkey), -15.)
        for p in ps:
            self.assertEqual(p.get_value(vykey), 0.)
            self.assertEqual(p.get_value(vzkey), 0.)

if __name__ == '__main__':
    IMP.test.main()
