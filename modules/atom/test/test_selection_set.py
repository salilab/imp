import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):
    """Test set operations on selections"""

    def test_union(self):
        """Test selection union"""
        def assert_ok(ps):
            self.assertEqual(len(ps), 11)
            for p in ps:
                a = IMP.atom.Atom(m, p)
                r = IMP.atom.Residue(a.get_parent())
                rind = r.get_index()
                self.assert_(a.get_atom_type() == IMP.atom.AT_CG or rind == 433)
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s1 = IMP.atom.Selection(h, residue_type=IMP.atom.ASP)
        s2 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG)
        for s in s1 | s2, s1.union(s2):
            ps = s.get_selected_particle_indexes()
            assert_ok(ps)
        # Test union update
        s1 |= s2
        ps = s1.get_selected_particle_indexes()
        assert_ok(ps)

    def test_intersection(self):
        """Test selection intersection"""
        def assert_ok(ps):
            self.assertEqual(len(ps), 1)
            for p in ps:
                a = IMP.atom.Atom(m, p)
                r = IMP.atom.Residue(a.get_parent())
                rind = r.get_index()
                self.assert_(a.get_atom_type() == IMP.atom.AT_CG \
                             and rind == 433)
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s1 = IMP.atom.Selection(h, residue_type=IMP.atom.ASP)
        s2 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG)
        for s in s1 & s2, s1.intersection(s2):
            ps = s.get_selected_particle_indexes()
            assert_ok(ps)
        # Test intersection update
        s1 &= s2
        ps = s1.get_selected_particle_indexes()
        assert_ok(ps)

if __name__ == '__main__':
    IMP.test.main()
