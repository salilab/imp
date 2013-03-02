import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Test the Charged decorator"""

    def test_create(self):
        """Check creation of Charged decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        # Can create decorators using a Vector3D and charge, or just charge
        # if the particle is already an XYZ
        IMP.atom.Charged.setup_particle(p, v, -0.5)
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p, v)
        IMP.atom.Charged.setup_particle(p, -0.5)
        p = IMP.Particle(m)
        if IMP.base.get_check_level() != IMP.base.NONE:
            self.assertRaises(IMP.base.UsageException,
                              IMP.atom.Charged.setup_particle, p, -0.5)

    def test_get_set(self):
        """Check get/set methods of Charged decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        c = IMP.atom.Charged.setup_particle(p, v, -0.5)
        self.assertAlmostEqual(c.get_charge(), -0.5, delta=1e-6)
        c.set_charge(2.5)
        self.assertAlmostEqual(c.get_charge(), 2.5, delta=1e-6)

    def test_show(self):
        """Check show method of Charged decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
        c = IMP.atom.Charged.setup_particle(p, v, -0.5)
        self.assertIn(' charge= ', str(c))

if __name__ == '__main__':
    IMP.test.main()
