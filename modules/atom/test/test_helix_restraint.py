import IMP
import IMP.atom
import IMP.test

class Tests(IMP.test.TestCase):
    def test_restraint(self):
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'),m)
        res = IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE)
        r = IMP.atom.HelixRestraint(res)
        self.assertEqual(r.get_number_of_dihedrals(),len(res)-2)
        self.assertEqual(r.get_number_of_bonds(),len(res)-4)
if __name__ == '__main__':
    IMP.test.main()
