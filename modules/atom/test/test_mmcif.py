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
        chain_ids = [IMP.atom.Chain(x).get_id()
                     for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual(chain_ids, ['', 'B', 'A'])
        self.assertEqual(len(m.get_particle_indexes()), 435)

    def test_read_multimodel(self):
        """Check reading a multimodel mmCIF file"""
        m = IMP.Model()

        #! read PDB
        mps = IMP.atom.read_multimodel_mmcif(
                                self.open_input_file("input.cif"), m)
        mp1, mp2 = mps
        chains1 = [IMP.atom.Chain(x)
                   for x in IMP.atom.get_by_type(mp1, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains1], ['', 'B', 'A'])
        chains2 = [IMP.atom.Chain(x)
                   for x in IMP.atom.get_by_type(mp2, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains2], [''])
        # Blank chain in the two models should be different particles
        self.assertNotEqual(chains1[0].get_particle_index(),
                            chains2[0].get_particle_index())
        self.assertEqual(len(m.get_particle_indexes()), 444)


if __name__ == '__main__':
    IMP.test.main()
