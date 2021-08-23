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
        with self.open_input_file("notapdb.pdb") as fh:
            self.assertRaises(IMP.ValueException,
                              IMP.atom.read_mmcif, fh, m)

    def test_read_pdb_or_mmcif(self):
        """Check reading mmCIF with read_pdb_or_mmcif"""
        m = IMP.Model()

        mp = IMP.atom.read_pdb_or_mmcif(
            self.get_input_file_name("input.cif"), m)
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual(len(chains), 3)
        self.assertEqual(len(m.get_particle_indexes()), 435)

    def test_read_pdb_or_mmcif_no_num(self):
        """Check reading mmCIF with read_pdb_or_mmcif, ignoring model num"""
        m = IMP.Model()

        mp = IMP.atom.read_pdb_or_mmcif(
            self.get_input_file_name("input.cif"), m,
            IMP.atom.NonWaterPDBSelector(), False)
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual(len(chains), 3)
        self.assertEqual(len(m.get_particle_indexes()), 441)

    def test_read(self):
        """Check reading an mmCIF file with one protein"""
        m = IMP.Model()

        #! read PDB
        with self.open_input_file("input.cif") as fh:
            mp = IMP.atom.read_mmcif(fh, m)
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual(len(m.get_particle_indexes()), 435)
        # Check chain IDs, residue indices and insertion codes (should use
        # author-provided values if available)
        self.assertEqual([c.get_id() for c in chains], ['', 'X', 'A'])
        rs = [IMP.atom.Residue(x)
              for x in IMP.atom.get_by_type(chains[0], IMP.atom.RESIDUE_TYPE)]
        indices = [r.get_index() for r in rs[:4]]
        self.assertEqual(indices, [-1, 0, 0, 4])
        inscodes = [r.get_insertion_code() for r in rs[:4]]
        self.assertEqual(inscodes, [' ', ' ', 'A', ' '])

    def test_read_pdb_no_num(self):
        """Check reading mmCIF with read_mmcif, ignoring model num"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(
            self.get_input_file_name("input.cif"), m,
            IMP.atom.NonWaterPDBSelector(), False)
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual(len(chains), 3)
        self.assertEqual(len(m.get_particle_indexes()), 441)

    def test_read_multimodel_pdb_or_mmcif(self):
        """Check reading mmCIF with read_multimodel_pdb_or_mmcif"""
        m = IMP.Model()

        mps = IMP.atom.read_multimodel_pdb_or_mmcif(
            self.get_input_file_name("input.cif"), m)
        mp1, mp2 = mps
        chains1 = [IMP.atom.Chain(x)
                   for x in IMP.atom.get_by_type(mp1, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains1], ['', 'X', 'A'])
        chains2 = [IMP.atom.Chain(x)
                   for x in IMP.atom.get_by_type(mp2, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains2], [''])

    def test_read_multimodel(self):
        """Check reading a multimodel mmCIF file"""
        m = IMP.Model()

        #! read PDB
        with self.open_input_file("input.cif") as fh:
            mps = IMP.atom.read_multimodel_mmcif(fh, m)
        mp1, mp2 = mps
        chains1 = [IMP.atom.Chain(x)
                   for x in IMP.atom.get_by_type(mp1, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains1], ['', 'X', 'A'])
        chains2 = [IMP.atom.Chain(x)
                   for x in IMP.atom.get_by_type(mp2, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains2], [''])
        # Blank chain in the two models should be different particles
        self.assertNotEqual(chains1[0].get_particle_index(),
                            chains2[0].get_particle_index())
        self.assertEqual(len(m.get_particle_indexes()), 444)

    def test_select_backbone(self):
        """Check reading an mmCIF file, only backbone"""
        m = IMP.Model()

        with self.open_input_file("input.cif") as fh:
            mp = IMP.atom.read_mmcif(fh, m, IMP.atom.BackbonePDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 278)
        # Only backbone atom types should have been read
        ats = frozenset(IMP.atom.Atom(x).get_atom_type().get_string()
                        for x in IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE))
        self.assertEqual(ats, frozenset(['CA', 'C', 'O', 'N']))

    def test_select_nitrogen(self):
        """Check reading an mmCIF file, only nitrogen atoms"""
        m = IMP.Model()

        with self.open_input_file("input.cif") as fh:
            mp = IMP.atom.read_mmcif(fh, m, IMP.atom.NPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 115)
        # Only nitrogens should have been read
        ats = frozenset(IMP.atom.Atom(x).get_atom_type().get_string()
                        for x in IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE))
        self.assertEqual(ats, frozenset(['N']))


if __name__ == '__main__':
    IMP.test.main()
