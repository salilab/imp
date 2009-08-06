import unittest
from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom

class ElementTableTest(IMP.test.TestCase):

    def test_element_mass(self):
        """Check assigning elements and masses to pdb with one protein"""
        m = IMP.Model()
        # read PDB, assign elements and masses
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterSelector())
        atoms = IMP.atom.get_by_type(mp, IMP.atom.Hierarchy.ATOM)
        # compute mass
        # mass is assigned using elements, so we are testing both here
        mass = 0
        for a in atoms:
            mass += a.get_mass()
        print mass
        self.assertInTolerance(mass, 13346.4, 1.0)

if __name__ == '__main__':
    unittest.main()
