import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra

def _make_test_decorator(m):
    p = IMP.Particle(m)
    v = IMP.algebra.Vector3D(1.0, 2.0, 3.0)
    s = IMP.algebra.Sphere3D(v, 4.0)
    IMP.core.XYZR.setup_particle(p, s)
    return IMP.atom.LennardJones.setup_particle(p, 0.5)


class Tests(IMP.test.TestCase):
    """Test the LennardJones decorator"""

    def test_create(self):
        """Check creation of LennardJones decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        s = IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1.0, 2.0, 3.0), 4.0)
        # Can create decorators using well depth, only if the particle is
        # already an XYZ
        p = IMP.Particle(m)
        IMP.core.XYZR.setup_particle(p, s)
        IMP.atom.LennardJones.setup_particle(p, 0.5)
        p = IMP.Particle(m)
        if IMP.base.get_check_level() != IMP.base.NONE:
            self.assertRaises(IMP.base.UsageException,
                              IMP.atom.LennardJones.setup_particle, p, 0.5)

    def test_get_set(self):
        """Check get/set methods of LennardJones decorators"""
        m = IMP.Model()
        c = _make_test_decorator(m)
        self.assertAlmostEqual(c.get_well_depth(), 0.5, delta=1e-6)
        c.set_well_depth(2.5)
        self.assertAlmostEqual(c.get_well_depth(), 2.5, delta=1e-6)

    def test_show(self):
        """Check show method of LennardJones decorators"""
        m = IMP.Model()
        c = _make_test_decorator(m)
        self.assertIn(' well depth= ', str(c))

if __name__ == '__main__':
    IMP.test.main()
