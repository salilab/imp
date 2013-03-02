import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_conversion(self):
        """Test conversion from atom.hierarchy"""
        m= IMP.Model()
        p =IMP.Particle(m)
        h= IMP.atom.Hierarchy.setup_particle(p)
        c= IMP.core.get_leaves(h)

if __name__ == '__main__':
    IMP.test.main()
