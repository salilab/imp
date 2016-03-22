import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_conversion(self):
        """Test conversion from atom.hierarchy"""
        m = IMP.Model()
        p = IMP.Particle(m)
        h = IMP.atom.Hierarchy.setup_particle(p)
        c = IMP.core.get_leaves(h)

    def test_get_is_valid(self):
        """Test Hierarchy::get_is_valid()"""
        m = IMP.Model()
        p = IMP.Particle(m)
        h = IMP.atom.Hierarchy.setup_particle(p)
        IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0,0,0))
        IMP.core.XYZR.setup_particle(p, 1.0)
        IMP.atom.Mass.setup_particle(p, 1.0)
        self.assertTrue(h.get_is_valid())
        self.assertTrue(h)

        # Default constructed Hierarchy is not valid
        nullh = IMP.atom.Hierarchy()
        self.assertFalse(nullh.get_is_valid())
        self.assertFalse(nullh)

if __name__ == '__main__':
    IMP.test.main()
