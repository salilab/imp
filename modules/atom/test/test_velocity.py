import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):

    def test_linear_velocity(self):
        """Test LinearVelocity decorator"""
        m = IMP.Model()
        p = IMP.Particle(m)
        self.assertFalse(IMP.atom.LinearVelocity.get_is_setup(p))
        d = IMP.atom.LinearVelocity.setup_particle(p)
        self.assertTrue(IMP.atom.LinearVelocity.get_is_setup(p))
        v = d.get_velocity()
        self.assertLess(IMP.algebra.get_distance(v,
                                                 IMP.algebra.Vector3D(0,0,0)),
                        1e-4)
        d.set_velocity(IMP.algebra.Vector3D(1,2,3))
        v = d.get_velocity()
        self.assertLess(IMP.algebra.get_distance(v,
                                                 IMP.algebra.Vector3D(1,2,3)),
                        1e-4)

    def test_angular_velocity(self):
        """Test AngularVelocity decorator"""
        m = IMP.Model()
        p = IMP.Particle(m)
        self.assertFalse(IMP.atom.AngularVelocity.get_is_setup(p))
        d = IMP.atom.AngularVelocity.setup_particle(p)
        self.assertTrue(IMP.atom.AngularVelocity.get_is_setup(p))
        v = d.get_velocity()
        self.assertLess(IMP.algebra.get_distance(v,
                                                 IMP.algebra.Vector4D(0,0,0,0)),
                        1e-4)
        d.set_velocity(IMP.algebra.Vector4D(1,2,3,4))
        v = d.get_velocity()
        self.assertLess(IMP.algebra.get_distance(v,
                                                 IMP.algebra.Vector4D(1,2,3,4)),
                        1e-4)

if __name__ == '__main__':
    IMP.test.main()
