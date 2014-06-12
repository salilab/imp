import StringIO
import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def _test_round_trip(self, name, selector):
        m = IMP.kernel.Model()
        p = IMP.atom.read_pdb(self.get_input_file_name(name),
                              m, selector)
        n1 = len(IMP.atom.get_by_type(p, IMP.atom.ATOM_TYPE))
        sout = StringIO.StringIO()
        IMP.atom.write_pdb(p, sout)
        sin = StringIO.StringIO(sout.getvalue())
        p2 = IMP.atom.read_pdb(sin, m, selector)
        n2 = len(IMP.atom.get_by_type(p2, IMP.atom.ATOM_TYPE))
        self.assertEqual(n1, n2)
        self.assertGreater(n1, 0)

    def test_bad_read(self):
        """Check that read_pdb behaves OK on invalid files"""
        m = IMP.kernel.Model()
        self.assertRaises(IMP.base.IOException,
                          IMP.atom.read_pdb, "notafile.pdb",
                          m)
        # we don't actually check if a file is a pdb or not
        # and can't conclude it is not due to not reading any atoms
        # as the selector may filter them all.
        self.assertRaises(IMP.base.ValueException,
                          IMP.atom.read_pdb,
                          self.open_input_file("notapdb.pdb"),
                          m)

    def test_read(self):
        """Check reading a pdb with one protein"""
        m = IMP.kernel.Model()

        #! read PDB
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        self.assertEqual(m.get_number_of_particles(), 1132)
        # IMP.atom.show_molecular_hierarchy(mp)
        IMP.atom.show(mp)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(len(bds), 1020)
        IMP.atom.add_radii(mp)
        IMP.atom.show_molecular_hierarchy(mp)

        m2 = IMP.kernel.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m2, IMP.atom.CAlphaPDBSelector())
        self.assertEqual(m2.get_number_of_particles(), 260)
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 129)
        IMP.atom.add_bonds(mp)
        bds = IMP.atom.get_internal_bonds(mp)
        self.assertEqual(len(bds), 0)

        # more selector testing
        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.BackbonePDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 516)

        mp = IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                               m, IMP.atom.NPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 129)

        # one more test for DNA
        mp = IMP.atom.read_pdb(self.open_input_file("single_dna.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 3011)

    def test_read_non_water(self):
        """Check that the default pdb reader skips waters"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m = IMP.kernel.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("protein_water.pdb"),
                               m)
        a = IMP.atom.get_leaves(mp)
        IMP.atom.write_pdb(mp, self.get_tmp_file_name("water_write.pdb"))
        self.assertEqual(len(a), 320)

    def test_read_non_hydrogen(self):
        """Check that the Hydrogen selector can identify all hydrogens"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m = IMP.kernel.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                               m, IMP.atom.HydrogenPDBSelector())
        a = IMP.atom.get_leaves(mp)
        self.assertEqual(len(a), 22)

    def test_sel_logic(self):
        """Test boolean logic selectors"""
        m = IMP.kernel.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                               m, IMP.atom.HydrogenPDBSelector())
        a = IMP.atom.get_leaves(mp)
        mpn = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                                m, IMP.atom.NotPDBSelector(IMP.atom.HydrogenPDBSelector()))
        an = IMP.atom.get_leaves(mpn)
        mpb = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                                m, IMP.atom.OrPDBSelector(IMP.atom.NotPDBSelector(IMP.atom.HydrogenPDBSelector()), IMP.atom.HydrogenPDBSelector()))
        ab = IMP.atom.get_leaves(mpb)
        self.assertEqual(len(ab), len(an) + len(a))
        mpb = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                                m, IMP.atom.AndPDBSelector(IMP.atom.HydrogenPDBSelector(), IMP.atom.ChainPDBSelector('L')))
        ab = IMP.atom.get_leaves(mpb)
        self.assertEqual(len(ab), 9)

    def test_pyimpl(self):
        """Test PDBSelectors implemented in Python"""
        class my_selector(IMP.atom.PDBSelector):

            def __init__(self):
                IMP.atom.PDBSelector.__init__(self, "my selector")

            def get_is_selected(self, ln):
                return ln.startswith("ATOM")

        m = IMP.kernel.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                               m, IMP.atom.ATOMPDBSelector())
        mp_py = IMP.atom.read_pdb(self.open_input_file("hydrogen.pdb"),
                                  m, my_selector())

        l = IMP.atom.get_leaves(mp)
        self.assertEqual(len(l), 24)
        l_py = IMP.atom.get_leaves(mp_py)
        self.assertEqual(len(l), len(l_py))

    def test_read_non_prob(self):
        """Check that problem lines are read properly"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m = IMP.kernel.Model()
        mp = IMP.atom.read_pdb(self.open_input_file("problem_lines.pdb"), m)
        a = IMP.atom.get_leaves(mp)
        self.assertEqual(len(a), 1)

    def test_read_one_model(self):
        """Check that only the first model is read"""
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("multimodel.pdb"), m,
                              IMP.atom.AllPDBSelector(), True)
        # print m.number
        ln = IMP.atom.get_leaves(h)
        print len(ln)
        self.assertLess(len(ln), 1000)

    def test_one_atom(self):
        """Test reading a PDB containing a single atom"""
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("single_atom.pdb"), m,
                              IMP.atom.AllPDBSelector(), True)
        # print m.number
        ln = IMP.atom.get_leaves(h)
        print len(ln)
        self.assertLess(len(ln), 1000)

    def test_indexes(self):
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("dna.pdb"), m)
        tn = self.get_tmp_file_name("out_dna.pdb")
        IMP.atom.write_pdb(h, tn)
        hp = IMP.atom.read_pdb(tn, m)
        lvs = IMP.atom.get_leaves(hp)
        self.assertEqual(IMP.atom.Atom(lvs[2]).get_input_index(), 3)

if __name__ == '__main__':
    IMP.test.main()
