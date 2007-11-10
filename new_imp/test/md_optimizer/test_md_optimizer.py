import unittest
import IMP.utils
import IMP.test, IMP

class MolecularDynamicsTests(IMP.test.IMPTestCase):
    """Test molecular dynamics optimizer"""

    def setUp(self):
        """Set up particles and optimizer"""

        self.model = IMP.Model()
        self.particles = []
        self.particles.append(IMP.utils.XYZParticle(self.model,
                                                    -43.0, 65.0, 93.0))
        self.md = IMP.MolecularDynamics()

    def test_non_xyz(self):
        """Should be unable to do MD on optimizable non-xyz attributes"""
        p = IMP.Particle()
        self.model.add_particle(p)
        p.add_attribute(IMP.FloatKey("attr"), 0.0, True)
        self.assertRaises(ValueError, self.md.optimize, self.model, 50, 300.)

    def test_basic_md(self):
        """Test basic MD"""
        self.md.optimize(self.model, 0, 300.0)
        keys = [IMP.FloatKey(x) for x in ("vx", "vy", "vz")]
        for p in self.particles:
            for key in keys:
                self.assert_(p.has_attribute(key))

if __name__ == '__main__':
    unittest.main()
