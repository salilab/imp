import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_bonded(self):
        """Check use of Selection """
        m = IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              m)
        pat=IMP.atom.Selection(mh, residue_index=10)
        print type(pat)
        got= pat.get_selected_particles()
        print [x.get_name() for x in got]
        self.assertGreater(len(got), 0)
        for g in [IMP.atom.Hierarchy(g) for g in got]:
            self.assertEqual(IMP.atom.Residue(g.get_parent()).get_index(), 10)

if __name__ == '__main__':
    IMP.test.main()
