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

    def test_triple_union(self):
        """Test selection triple union"""
        def assert_ok(ps):
            self.assertEqual(len(ps), 12)
            for p in ps:
                a = IMP.atom.Atom(m, p)
                at = a.get_atom_type()
                r = IMP.atom.Residue(a.get_parent())
                rind = r.get_index()
                self.assert_(at == IMP.atom.AT_CG or at == IMP.atom.AT_CG1
                             or rind == 433)
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s1 = IMP.atom.Selection(h, residue_type=IMP.atom.ASP)
        s2 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG)
        s3 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG1)
        for s in s1 | s2 | s3, s1.union(s2).union(s3):
            ps = s.get_selected_particle_indexes()
            assert_ok(ps)
        # Test union update
        s1 |= s2
        s1 |= s3
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

    def test_symmetric_difference(self):
        """Test selection symmetric difference"""
        def assert_ok(ps):
            self.assertEqual(len(ps), 10)
            for p in ps:
                a = IMP.atom.Atom(m, p)
                r = IMP.atom.Residue(a.get_parent())
                rind = r.get_index()
                self.assert_((a.get_atom_type() == IMP.atom.AT_CG) \
                             ^ (rind == 433))
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s1 = IMP.atom.Selection(h, residue_type=IMP.atom.ASP)
        s2 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG)
        for s in s1 ^ s2, s1.symmetric_difference(s2):
            ps = s.get_selected_particle_indexes()
            assert_ok(ps)
        # Test symmetric difference update
        s1 ^= s2
        ps = s1.get_selected_particle_indexes()
        assert_ok(ps)

    def test_difference(self):
        """Test selection difference"""
        def assert_ok(ps):
            self.assertEqual(len(ps), 7)
            for p in ps:
                a = IMP.atom.Atom(m, p)
                r = IMP.atom.Residue(a.get_parent())
                rind = r.get_index()
                self.assert_(a.get_atom_type() != IMP.atom.AT_CG
                             and rind == 433)
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s1 = IMP.atom.Selection(h, residue_type=IMP.atom.ASP)
        s2 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG)
        for s in s1 - s2, s1.difference(s2):
            ps = s.get_selected_particle_indexes()
            assert_ok(ps)
        # Test difference update
        s1 -= s2
        ps = s1.get_selected_particle_indexes()
        assert_ok(ps)

    def test_multiple(self):
        """Test combination of multiple selections"""
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s = IMP.atom.Selection(h, residue_type=IMP.atom.PHE) \
              - (IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG) \
                 | IMP.atom.Selection(h, terminus=IMP.atom.Selection.C))
        ps = s.get_selected_particle_indexes()
        self.assertEqual(len(ps), 19)
        for p in ps:
            a = IMP.atom.Atom(m, p)
            r = IMP.atom.Residue(a.get_parent())
            rind = r.get_index()
            at = a.get_atom_type()
            self.assert_(at != IMP.atom.AT_CG
                         and (rind == 437 or rind == 440)
                         and not (rind == 440 and at == IMP.atom.AT_C))

    def test_modify_after(self):
        """Modifying a selection after set op should not affect the result"""
        def assert_ok(s):
            ps = s.get_selected_particle_indexes()
            self.assertEqual(len(ps), 4)
            for p in ps:
                a = IMP.atom.Atom(m, p)
                at = a.get_atom_type()
                self.assert_(at == IMP.atom.AT_CG)
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.kernel.Model()
        h = IMP.atom.read_pdb(self.open_input_file("mini.pdb"), m)
        s1 = IMP.atom.Selection(h, atom_type=IMP.atom.AT_CG)
        s2 = IMP.atom.Selection(h)
        s3 = s1 & s2
        assert_ok(s3)
        s1.set_residue_type(IMP.atom.PHE)
        assert_ok(s3)
        s2.set_residue_type(IMP.atom.ALA)
        assert_ok(s3)
        s1 &= IMP.atom.Selection(h, atom_type=IMP.atom.AT_C)
        assert_ok(s3)
        s2 &= IMP.atom.Selection(h, atom_type=IMP.atom.AT_C)
        assert_ok(s3)

if __name__ == '__main__':
    IMP.test.main()
