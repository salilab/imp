import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    """Test the Residue class and related functions"""

    def test_rna_dna_protein(self):
        """Check Residue get_is_*() methods"""
        m = IMP.Model()
        user = IMP.atom.ResidueType('foo')
        for typ, rna, dna, protein in ((IMP.atom.ALA, False, False, True),
                                       (IMP.atom.MSE, False, False, True),
                                       (IMP.atom.ADE, True, False, False),
                                       (IMP.atom.DADE, False, True, False),
                                       (IMP.atom.HOH, False, False, False),
                                       # user-added residues are also ligands
                                       (user, False, False, False)):
            p = IMP.Particle(m)
            r = IMP.atom.Residue.setup_particle(p, typ)
            self.assertEqual(r.get_is_rna(), rna)
            self.assertEqual(r.get_is_dna(), dna)
            self.assertEqual(r.get_is_protein(), protein)

    def test_type_trigger(self):
        """Check Residue type changed Model trigger"""
        m = IMP.Model()
        tk = IMP.atom.Residue.get_type_changed_key()
        self.assertEqual(m.get_trigger_last_updated(tk), 0)
        p = IMP.Particle(m)
        r = IMP.atom.Residue.setup_particle(p, IMP.atom.ALA)
        self.assertEqual(m.get_trigger_last_updated(tk), 1)
        self.assertEqual(m.get_age(), 1)

        # Update should increase model age but not change trigger
        m.update()
        self.assertEqual(m.get_trigger_last_updated(tk), 1)
        self.assertEqual(m.get_age(), 2)

        # Changing type should update the trigger
        r.set_residue_type(IMP.atom.CYS)
        self.assertEqual(m.get_trigger_last_updated(tk), 2)


if __name__ == '__main__':
    IMP.test.main()
