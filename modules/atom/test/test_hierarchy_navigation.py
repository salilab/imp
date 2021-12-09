import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_read(self):
        """Check get_next_residue() method"""
        m = IMP.Model()

        #! read PDB
        with self.open_input_file("input.pdb") as fh:
            mp = IMP.atom.read_pdb(fh, m, IMP.atom.NonWaterPDBSelector())
        res = IMP.atom.get_by_type(mp, IMP.atom.RESIDUE_TYPE)
        nres = IMP.atom.get_next_residue(IMP.atom.Residue(res[0]))

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_no_chain(self):
        """Check failure when Residue has no Chain parent"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r = IMP.atom.Residue.setup_particle(m, p, IMP.atom.GLY, 42)
        self.assertRaises(IMP.UsageException, IMP.atom.get_next_residue, r)
        self.assertRaises(IMP.UsageException, IMP.atom.get_previous_residue, r)

    def test_fragment(self):
        """Check finding next/previous residue in more complex hierarchy"""
        m = IMP.Model()
        # should work for residues that are direct children of chain, or
        # children of fragments
        chain = IMP.atom.Chain.setup_particle(IMP.Particle(m), 'A')
        frag1 = IMP.atom.Fragment.setup_particle(IMP.Particle(m))
        frag2 = IMP.atom.Fragment.setup_particle(IMP.Particle(m))
        r3 = IMP.atom.Residue.setup_particle(IMP.Particle(m), IMP.atom.GLY, 3)
        r4 = IMP.atom.Residue.setup_particle(IMP.Particle(m), IMP.atom.GLY, 4)
        r5 = IMP.atom.Residue.setup_particle(IMP.Particle(m), IMP.atom.GLY, 5)
        r6 = IMP.atom.Residue.setup_particle(IMP.Particle(m), IMP.atom.GLY, 6)
        h = IMP.atom.Hierarchy(chain)
        for c in [r3, frag1, frag2]:
            h.add_child(c)
        IMP.atom.Hierarchy(frag1).add_child(r4)
        IMP.atom.Hierarchy(frag1).add_child(r5)
        IMP.atom.Hierarchy(frag2).add_child(r6)
        def assert_residue_index(func, start_res, exp_index):
            r = func(start_res)
            self.assertTrue(IMP.atom.Residue.get_is_setup(r))
            self.assertEqual(IMP.atom.Residue(r).get_index(), exp_index)

        assert_residue_index(IMP.atom.get_next_residue, r3, 4)
        assert_residue_index(IMP.atom.get_next_residue, r4, 5)
        assert_residue_index(IMP.atom.get_next_residue, r5, 6)
        self.assertFalse(IMP.atom.get_next_residue(r6))

        self.assertFalse(IMP.atom.get_previous_residue(r3))
        assert_residue_index(IMP.atom.get_previous_residue, r4, 3)
        assert_residue_index(IMP.atom.get_previous_residue, r5, 4)
        assert_residue_index(IMP.atom.get_previous_residue, r6, 5)


if __name__ == '__main__':
    IMP.test.main()
