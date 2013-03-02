import StringIO
import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):
    def _get_index(self, l):
        h=IMP.atom.Hierarchy(l[0])
        p= IMP.atom.Residue(h.get_parent())
        return p.get_index()
    def test_one(self):
        """Test selection of N and C termini"""
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.open_input_file("input.pdb"), m)
        cterm= IMP.atom.Selection(h, terminus=IMP.atom.Selection.C)
        nterm= IMP.atom.Selection(h, terminus=IMP.atom.Selection.N)
        print cterm.get_selected_particles()
        print nterm.get_selected_particles()
        cterm= IMP.atom.Selection(h, terminus=IMP.atom.Selection.C)
        self.assertEqual(len(cterm.get_selected_particles()), 1)
        self.assertEqual(len(nterm.get_selected_particles()), 1)
        self.assertEqual(self._get_index(cterm.get_selected_particles()), 129)
        self.assertEqual(self._get_index(nterm.get_selected_particles()), 1)
if __name__ == '__main__':
    IMP.test.main()
