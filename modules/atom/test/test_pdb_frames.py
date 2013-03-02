import StringIO
import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_read_one_model(self):
        """Reading of successive models from a pdb"""
        m = IMP.Model()
        pdb=self.open_input_file("multimodel.pdb")
        h= IMP.atom.read_pdb(pdb, m)
        atom0= IMP.core.XYZ(IMP.atom.get_leaves(h)[0])
        self.assertAlmostEqual(atom0.get_x(), 24, delta=1)
        IMP.atom.read_pdb(pdb, 3, h)
        self.assertAlmostEqual(atom0.get_x(), 10, delta=1)
        self.assertRaises(ValueError, IMP.atom.read_pdb, pdb, 21, h)

if __name__ == '__main__':
    IMP.test.main()
