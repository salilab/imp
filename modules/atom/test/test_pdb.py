import unittest
from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom

class PDBReadWriteTest(IMP.test.TestCase):

    def test_read(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterSelector())
        self.assertEqual(m.get_number_of_particles(), 1132)
        #IMP.atom.show_molecular_hierarchy(mp)
        mp.show()
        IMP.atom.show(mp)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(bds.size(), 1020)
        IMP.atom.add_radii(mp)
        IMP.atom.show_molecular_hierarchy(mp)

        m2 = IMP.Model()
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m2, IMP.atom.CAlphaSelector())
        self.assertEqual(m2.get_number_of_particles(), 260)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(bds.size(), 0)

    def test_write(self):
        """Simple test of writing a PDB"""
        m = IMP.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.CAlphaSelector())
        s = StringIO()
        IMP.atom.write_pdb(mp, s)
        self.assertEqual(s.getvalue().count('\n'), 129)

if __name__ == '__main__':
    unittest.main()
