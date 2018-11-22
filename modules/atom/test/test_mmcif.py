from __future__ import print_function
import io
import IMP
import IMP.test
import IMP.atom
import IMP.core


class Tests(IMP.test.TestCase):

    def test_bad_read(self):
        """Check that read_mmcif behaves OK on invalid files"""
        m = IMP.Model()
        self.assertRaises(IMP.IOException,
                          IMP.atom.read_mmcif, "notafile.pdb",
                          m)
        self.assertRaises(IMP.ValueException,
                          IMP.atom.read_mmcif,
                          self.open_input_file("notapdb.pdb"),
                          m)

    def test_read(self):
        """Check reading an mmCIF file with one protein"""
        m = IMP.Model()

        #! read PDB
        mp = IMP.atom.read_mmcif(self.open_input_file("input.cif"), m)
        self.assertEqual(len(m.get_particle_indexes()), 430)


if __name__ == '__main__':
    IMP.test.main()
