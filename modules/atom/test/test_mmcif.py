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

    def test_multiple_ligands(self):
        """Check handling of multiple ligands"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(
            self.get_input_file_name("long_ligands.cif"), m)

        self.assertEqual(len(m.get_particle_indexes()), 47)
        # Should use author-provided information; everything is in chain A,
        # numbered sequentially
        chains = IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)
        self.assertEqual(len(chains), 1)
        self.assertEqual(IMP.atom.Chain(chains[0]).get_id(), 'A')
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(len(residues), 4)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [279, 301, 302, 303])
        # Make sure that ligand names are not restricted to 3 characters
        self.assertEqual([IMP.atom.Residue(x).get_residue_type().get_string()
                          for x in residues],
                         ['LYS', 'CA', '7ZTVU', '7ZTVU'])

    def test_multiple_ligands_no_auth(self):
        """Check handling of multiple ligands with no author-provided info"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(
            self.get_input_file_name("long_ligands_no_auth.cif"), m)

        self.assertEqual(len(m.get_particle_indexes()), 53)
        # Should use internal cif information; each ligand is in a separate
        # chain, and has default seq_id ("." in mmCIF, 1 in IMP)
        chains = IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)
        self.assertEqual(len(chains), 4)
        self.assertEqual([IMP.atom.Chain(x).get_id() for x in chains],
                         ['A', 'B', 'C', 'D'])
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        self.assertEqual(len(residues), 4)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [261, 1, 1, 1])
        # Make sure that ligand names are not restricted to 3 characters
        self.assertEqual([IMP.atom.Residue(x).get_residue_type().get_string()
                          for x in residues],
                         ['LYS', 'CA', '7ZTVU', '7ZTVU'])

    def test_non_alt_selector(self):
        """Check NonAlternativePDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('nonalttest.cif'), m,
                                 IMP.atom.NonAlternativePDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residue #3 should be rejected because its alt_loc is "B" (only
        # blank or "A" alt_loc should be accepted)
        # Residue #4 should be rejected because alt_loc is "AA". Previous
        # versions of IMP would pass this because it would be truncated to
        # one character, "A".
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1, 2, 5, 6])

    def test_atom_selector(self):
        """Check ATOMPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtest.cif'), m,
                                 IMP.atom.ATOMPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [68, 287, 287, 287, 287])

    def test_calpha_selector(self):
        """Check CAlphaPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.CAlphaPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Only residue 1 contains a C-alpha (residue 6 is calcium, should
        # be rejected)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1])

    def test_cbeta_selector(self):
        """Check CBetaPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.CBetaPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Only residue 2 contains a C-beta
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [2])

    def test_atom_type_selector(self):
        """Check AtomTypePDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.AtomTypePDBSelector(['CA', 'O']))
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residue 1 is C-alpha; residue 4 is O; residue 7 is "CA" (calcium)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1, 4, 7])

    def test_residue_type_selector(self):
        """Check ResidueTypePDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.ResidueTypePDBSelector(['ALA']))
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residues 1 and 4 are ALA
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1, 4])

    def test_c_selector(self):
        """Check CPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.CPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Only residue 3 contains a C atom
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [3])

    def test_n_selector(self):
        """Check NPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.NPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Only residue 5 contains a N atom
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [5])

    def test_p_selector(self):
        """Check PPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.PPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Only residue 6 contains a P atom
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [6])

    def test_all_selector(self):
        """Check AllPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('nonalttest.cif'), m,
                                 IMP.atom.AllPDBSelector())
        # All alternative locations should be selected
        self.assertEqual(len(IMP.atom.get_leaves(mp)), 6)

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.AllPDBSelector())
        # Both ATOM and HETATM records should be selected
        self.assertEqual(len(IMP.atom.get_leaves(mp)), 7)

    def test_water_selector(self):
        """Check WaterPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('water.cif'), m,
                                 IMP.atom.WaterPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residues 3, 4 are waters
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [3, 4])

    def test_hydrogen_selector(self):
        """Check HydrogenPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('water.cif'), m,
                                 IMP.atom.HydrogenPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residue 2 only contains hydrogen
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [2])

    def test_non_water_non_hydrogen_selector(self):
        """Check NonWaterNonHydrogenPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('water.cif'), m,
                                 IMP.atom.NonWaterNonHydrogenPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residue 1 contains CA (not water or hydrogen)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1])

    def test_non_hydrogen_selector(self):
        """Check NonHydrogenPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('water.cif'), m,
                                 IMP.atom.NonHydrogenPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residue 1 contains CA, 3 and 4 are water oxygens
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1, 3, 4])

    def test_non_water_selector(self):
        """Check NonWaterPDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('water.cif'), m,
                                 IMP.atom.NonWaterPDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1, 2])

    def test_backbone_selector(self):
        """Check BackbonePDBSelector when reading mmCIF files"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('atomtype.cif'), m,
                                 IMP.atom.BackbonePDBSelector())
        residues = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        # Residue 2 is excluded because it only contains CB (non-backbone);
        # residue 6 is excluded because its "CA" is calcium, not C-alpha
        self.assertEqual([IMP.atom.Residue(x).get_index() for x in residues],
                         [1, 3, 4, 5])

    def test_chain_selector(self):
        """Check reading single chain from an mmCIF file"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["K"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['K'])

        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["7"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['7'])

        # If no auth-provided chain, select by asym_id
        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["B"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['B'])

    def test_chain_selector_multi_char(self):
        """Check reading single chain with multi-char ID from an mmCIF file"""
        m = IMP.Model()

        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["Z", "K"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['K'])

        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["ZK"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['ZK'])

        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["Z7"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['Z7'])

        # If no auth-provided chain, select by asym_id
        mp = IMP.atom.read_mmcif(self.get_input_file_name('chaintest.cif'), m,
                                 IMP.atom.ChainPDBSelector(["ZB"]))
        chains = [IMP.atom.Chain(x)
                  for x in IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)]
        self.assertEqual([c.get_id() for c in chains], ['ZB'])


if __name__ == '__main__':
    IMP.test.main()
