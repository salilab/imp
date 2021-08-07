import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    """Test the Atom class and related functions"""

    def test_type_trigger(self):
        """Check Atom type changed Model trigger"""
        m = IMP.Model()
        tk = IMP.atom.Residue.get_type_changed_key()
        self.assertEqual(m.get_trigger_last_updated(tk), 0)
        p = IMP.Particle(m)
        a = IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
        self.assertEqual(m.get_trigger_last_updated(tk), 1)
        self.assertEqual(m.get_age(), 1)

        # Update should increase model age but not change trigger
        m.update()
        self.assertEqual(m.get_trigger_last_updated(tk), 1)
        self.assertEqual(m.get_age(), 2)

        # Changing type should update the trigger
        a.set_atom_type(IMP.atom.AT_C)
        self.assertEqual(m.get_trigger_last_updated(tk), 2)


if __name__ == '__main__':
    IMP.test.main()
