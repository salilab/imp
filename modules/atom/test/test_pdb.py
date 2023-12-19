from __future__ import print_function
import io
import IMP
import IMP.test
import IMP.atom
import IMP.core


class Tests(IMP.test.TestCase):

    def _test_round_trip(self, name, selector):
        m = IMP.Model()
        p = IMP.atom.read_pdb(self.get_input_file_name(name),
                              m, selector)
        n1 = len(IMP.atom.get_by_type(p, IMP.atom.ATOM_TYPE))
        sout = io.BytesIO()
        IMP.atom.write_pdb(p, sout)
        sin = io.BytesIO(sout.getvalue())
        p2 = IMP.atom.read_pdb(sin, m, selector)
        n2 = len(IMP.atom.get_by_type(p2, IMP.atom.ATOM_TYPE))
        self.assertEqual(n1, n2)
        self.assertGreater(n1, 0)

    def test_bad_read(self):
        """Check that read_pdb behaves OK on invalid files"""
        m = IMP.Model()
        self.assertRaises(IMP.IOException,
                          IMP.atom.read_pdb, "notafile.pdb",
                          m)
        # we don't actually check if a file is a pdb or not
        # and can't conclude it is not due to not reading any atoms
        # as the selector may filter them all.
        with self.open_input_file("notapdb.pdb") as fh:
            self.assertRaises(IMP.ValueException, IMP.atom.read_pdb, fh, m)

    def test_selector_standard_object_methods(self):
        """Check standard object methods of selectors"""
        self.check_standard_object_methods(IMP.atom.NonWaterPDBSelector())
        self.check_standard_object_methods(IMP.atom.NPDBSelector())

    def test_invalid(self):
        """Check handling of invalid format PDB files"""
        m = IMP.Model()
        with self.open_input_file("invalid.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NonWaterPDBSelector())

    def test_read_mmcif_or_pdb(self):
        """Check reading a PDB with read_pdb_or_mmcif()"""
        m = IMP.Model()

        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb_or_mmcif(
                fh, m, IMP.atom.NonWaterPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 1133)

    def test_read(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        #! read PDB
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NonWaterPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 1133)
        # IMP.atom.show_molecular_hierarchy(mp)
        IMP.atom.show(mp)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(len(bds), 1020)
        IMP.atom.add_radii(mp)
        IMP.atom.show_molecular_hierarchy(mp)

    def test_calpha_pdb_selector(self):
        """Test CAlphaPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.CAlphaPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 261)
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 129)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(len(bds), 0)

    def test_cbeta_pdb_selector(self):
        """Test CBetaPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.CBetaPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 237)
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 117)

    def test_c_pdb_selector(self):
        """Test CPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.CPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 261)
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 129)

    def test_water_pdb_selector(self):
        """Test WaterPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.WaterPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 205)
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 101)

    def test_non_water_non_hydrogen_pdb_selector(self):
        """Test NonWaterNonHydrogenPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(
                fh, m, IMP.atom.NonWaterNonHydrogenPDBSelector())
        self.assertEqual(len(m.get_particle_indexes()), 1133)
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 1001)

    def test_backbone_pdb_selector(self):
        """Test BackbonePDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.BackbonePDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 516)

    def test_n_pdb_selector(self):
        """Test NPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 129)

    def test_non_water_pdb_selector_dna(self):
        """Test NonWaterPDBSelector with DNA"""
        m = IMP.Model()
        with self.open_input_file("single_dna.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NonWaterPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 3011)

    def test_p_pdb_selector(self):
        """Test PPDBSelector"""
        m = IMP.Model()
        with self.open_input_file("single_dna.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.PPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 146)

    def test_read_non_water(self):
        """Check that the default pdb reader skips waters"""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        with self.open_input_file("protein_water.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m)
        a = IMP.atom.get_leaves(mp)
        IMP.atom.write_pdb(mp, self.get_tmp_file_name("water_write.pdb"))
        self.assertEqual(len(a), 320)

    def test_read_hydrogen(self):
        """Check that the Hydrogen selector can identify all hydrogens"""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        with self.open_input_file("hydrogen.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.HydrogenPDBSelector())
        a = IMP.atom.get_leaves(mp)
        self.assertEqual(len(a), 22)

    def test_read_non_hydrogen(self):
        """Check NonHydrogenPDBSelector"""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        with self.open_input_file("hydrogen.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NonHydrogenPDBSelector())
        a = IMP.atom.get_leaves(mp)
        self.assertEqual(len(a), 3)

    def test_atom_type(self):
        """Test AtomTypePDBSelector"""
        m = IMP.Model()
        with self.open_input_file("hydrogen.pdb") as fh:
            mp = IMP.atom.read_pdb(
                fh, m, IMP.atom.AtomTypePDBSelector(["HA", "3HE"]))
        self.assertEqual(len(IMP.atom.get_leaves(mp)), 3)

    def test_residue_type(self):
        """Test ResidueTypePDBSelector"""
        m = IMP.Model()
        with self.open_input_file("hydrogen.pdb") as fh:
            mp = IMP.atom.read_pdb(
                fh, m, IMP.atom.ResidueTypePDBSelector(["MET"]))
        self.assertEqual(len(IMP.atom.get_leaves(mp)), 22)

    def test_atom_pdb_selector(self):
        """Test ATOMPDBSelector"""
        m = IMP.Model()
        mp = IMP.atom.read_pdb(self.get_input_file_name("protein_water.pdb"),
                               m, IMP.atom.ATOMPDBSelector())
        self.assertEqual(len(IMP.atom.get_leaves(mp)), 300)

    def test_sel_logic(self):
        """Test boolean logic selectors"""
        m = IMP.Model()
        with self.open_input_file("hydrogen.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.HydrogenPDBSelector())
        a = IMP.atom.get_leaves(mp)
        with self.open_input_file("hydrogen.pdb") as fh:
            mpn = IMP.atom.read_pdb(
                fh, m, IMP.atom.NotPDBSelector(IMP.atom.HydrogenPDBSelector()))
        an = IMP.atom.get_leaves(mpn)
        for s in (IMP.atom.OrPDBSelector(IMP.atom.NotPDBSelector(IMP.atom.HydrogenPDBSelector()), IMP.atom.HydrogenPDBSelector()),
              ~IMP.atom.HydrogenPDBSelector() | IMP.atom.HydrogenPDBSelector()):
            with self.open_input_file("hydrogen.pdb") as fh:
                mpb = IMP.atom.read_pdb(fh, m, s)
            ab = IMP.atom.get_leaves(mpb)
            self.assertEqual(len(ab), len(an) + len(a))
        for s in (IMP.atom.AndPDBSelector(IMP.atom.HydrogenPDBSelector(),
                                          IMP.atom.ChainPDBSelector(['L'])),
                  IMP.atom.HydrogenPDBSelector()
                        & IMP.atom.ChainPDBSelector(['L'])):
            with self.open_input_file("hydrogen.pdb") as fh:
                mpb = IMP.atom.read_pdb(fh, m, s)
            ab = IMP.atom.get_leaves(mpb)
            self.assertEqual(len(ab), 9)
        for s in (IMP.atom.XorPDBSelector(IMP.atom.HydrogenPDBSelector(),
                                          IMP.atom.ChainPDBSelector(['L'])),
                  IMP.atom.HydrogenPDBSelector()
                        ^ IMP.atom.ChainPDBSelector(['L'])):
            with self.open_input_file("hydrogen.pdb") as fh:
                mpb = IMP.atom.read_pdb(fh, m, s)
            ab = IMP.atom.get_leaves(mpb)
            self.assertEqual(len(ab), 14)
        for s in (IMP.atom.AndPDBSelector(
                      IMP.atom.HydrogenPDBSelector(),
                      IMP.atom.NotPDBSelector(
                          IMP.atom.ChainPDBSelector(['L']))),
                  IMP.atom.HydrogenPDBSelector()
                        - IMP.atom.ChainPDBSelector(['L'])):
            with self.open_input_file("hydrogen.pdb") as fh:
                mpb = IMP.atom.read_pdb(fh, m, s)
            ab = IMP.atom.get_leaves(mpb)
            self.assertEqual(len(ab), 13)

    def test_pyimpl(self):
        """Test PDBSelectors implemented in Python"""
        class my_selector(IMP.atom.PDBSelector):

            def __init__(self):
                IMP.atom.PDBSelector.__init__(self, "my selector")

            def get_is_selected(self, record):
                return record.get_is_atom()

        m = IMP.Model()
        with self.open_input_file("hydrogen.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.ATOMPDBSelector())
        with self.open_input_file("hydrogen.pdb") as fh:
            mp_py = IMP.atom.read_pdb(fh, m, my_selector())

        l = IMP.atom.get_leaves(mp)
        self.assertEqual(len(l), 25)
        l_py = IMP.atom.get_leaves(mp_py)
        self.assertEqual(len(l), len(l_py))

    def test_read_non_prob(self):
        """Check that problem lines are read properly"""
        IMP.set_log_level(IMP.VERBOSE)
        m = IMP.Model()
        with self.open_input_file("problem_lines.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m)
        a = IMP.atom.get_leaves(mp)
        self.assertEqual(len(a), 1)

    def test_read_multimodel_pdb_or_mmcif(self):
        """Check reading PDB with read_multimodel_pdb_or_mmcif"""
        m = IMP.Model()
        with self.open_input_file("multimodel.pdb") as fh:
            h = IMP.atom.read_multimodel_pdb_or_mmcif(
                fh, m, IMP.atom.AllPDBSelector())
        ln = IMP.atom.get_leaves(h)
        self.assertEqual(len(ln), 19740)

    def test_read_one_model(self):
        """Check that only the first model is read"""
        m = IMP.Model()
        with self.open_input_file("multimodel.pdb") as fh:
            h = IMP.atom.read_pdb(fh, m, IMP.atom.AllPDBSelector(), True)
        self.assertEqual(len(IMP.atom.get_leaves(h)), 987)
        self.assertEqual(len(IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)),
                         1)

    def test_read_combine_models(self):
        """Check read_pdb() reading multimodel into single hierarchy"""
        m = IMP.Model()
        with self.open_input_file("multimodel.pdb") as fh:
            h = IMP.atom.read_pdb(fh, m, IMP.atom.AllPDBSelector(), False)
        self.assertEqual(len(IMP.atom.get_leaves(h)), 19740)
        self.assertEqual(len(IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)),
                         1)

    def test_one_atom(self):
        """Test reading a PDB containing a single atom"""
        m = IMP.Model()
        with self.open_input_file("single_atom.pdb") as fh:
            h = IMP.atom.read_pdb(fh, m, IMP.atom.AllPDBSelector(), True)
        # print m.number
        ln = IMP.atom.get_leaves(h)
        print(len(ln))
        self.assertLess(len(ln), 1000)

    def test_indexes(self):
        m = IMP.Model()
        with self.open_input_file("dna.pdb") as fh:
            h = IMP.atom.read_pdb(fh, m)
        tn = self.get_tmp_file_name("out_dna.pdb")
        IMP.atom.write_pdb(h, tn)
        hp = IMP.atom.read_pdb(tn, m)
        lvs = IMP.atom.get_leaves(hp)
        self.assertEqual(IMP.atom.Atom(lvs[2]).get_input_index(), 3)

    def test_provenance(self):
        """Test that StructureProvenance is set"""
        m = IMP.Model()
        fname = self.get_input_file_name("hydrogen.pdb")
        mp = IMP.atom.read_pdb(fname, m)
        chains = IMP.atom.get_by_type(mp, IMP.atom.CHAIN_TYPE)
        self.assertEqual(len(chains), 5)
        for c in chains:
            self.assertTrue(IMP.core.Provenanced.get_is_setup(c))
            p = IMP.core.Provenanced(c).get_provenance()
            self.assertTrue(IMP.core.StructureProvenance.get_is_setup(p))
            sp = IMP.core.StructureProvenance(p)
            self.assertEqual(sp.get_filename(), fname)
            self.assertEqual(sp.get_chain_id(), IMP.atom.Chain(c).get_id())
            self.assertEqual(sp.get_residue_offset(), 0)


if __name__ == '__main__':
    IMP.test.main()
