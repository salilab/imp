import unittest
from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom

class PDBReadWriteTest(IMP.test.TestCase):
    def test_bad_read(self):
        """Check that read_pdb behaves OK on invalid files"""
        m= IMP.Model()
        self.assertRaises(IOError,
                          IMP.atom.read_pdb,
                          self.open_input_file("notapdb.pdb"),
                          m)

    def test_read(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        self.assertEqual(m.get_number_of_particles(), 1132)
        #IMP.atom.show_molecular_hierarchy(mp)
        IMP.atom.show(mp)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(bds.size(), 1020)
        IMP.atom.add_radii(mp)
        IMP.atom.show_molecular_hierarchy(mp)

        m2 = IMP.Model()
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m2, IMP.atom.CAlphaPDBSelector())
        self.assertEqual(m2.get_number_of_particles(), 260)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(bds.size(), 0)
    def test_read_het(self):
        """Check reading a pdb with one protein and a hetatm"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("1DQK.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        print m.get_number_of_particles()
        #self.assertEqual(m.get_number_of_particles(), 1132)
        #IMP.atom.show_molecular_hierarchy(mp)
        IMP.atom.show(mp)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        #self.assertEqual(bds.size(), 1020)
        IMP.atom.add_radii(mp)
        IMP.atom.show_molecular_hierarchy(mp)
    def test_read_non_water(self):
        """Check that the default pdb reader skips waters"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        mp= IMP.atom.read_pdb(self.open_input_file("protein_water.pdb"),
                              m)
        a= IMP.atom.get_leaves(mp)
        IMP.atom.write_pdb(mp, self.get_tmp_file_name("water_write.pdb"))
        self.assertEqual(len(a), 13328)
    def test_write(self):
        """Simple test of writing a PDB"""
        m = IMP.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.CAlphaPDBSelector())
        s = StringIO()
        IMP.atom.write_pdb(mp, s)
        self.assertEqual(s.getvalue().count('\n'), 129)

    def test_regression(self):
        """Test reading previously problematic pdbs"""
        m= IMP.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("regression_0.pdb"),
                               m)

if __name__ == '__main__':
    unittest.main()
