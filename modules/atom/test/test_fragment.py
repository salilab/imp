import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Check Fragment """
        m = IMP.Model()
        p=IMP.Particle(m)
        f= IMP.atom.Fragment.create(p)
        rs= IMP.Ints()
        rs.append(0)
        rs.append(1)
        rs.append(2)
        rs.append(6)
        f.set_residue_indexes(rs)
        self.assert_(f.get_contains_residue(0))
        self.assert_(f.get_contains_residue(1))
        self.assert_(f.get_contains_residue(2))
        self.assert_(f.get_contains_residue(6))
        self.assert_(not f.get_contains_residue(5))

if __name__ == '__main__':
    unittest.main()
