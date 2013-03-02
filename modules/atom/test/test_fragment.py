import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_bonded(self):
        """Check Fragment """
        m = IMP.Model()
        p=IMP.Particle(m)
        f= IMP.atom.Fragment.setup_particle(p)
        rs=[0,1,2,6]
        f.set_residue_indexes(rs)
        self.assertTrue(f.get_contains_residue(0))
        self.assertTrue(f.get_contains_residue(1))
        self.assertTrue(f.get_contains_residue(2))
        self.assertTrue(f.get_contains_residue(6))
        self.assertFalse(f.get_contains_residue(5))

if __name__ == '__main__':
    IMP.test.main()
