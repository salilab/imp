import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Check clone MHD """
        m = IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name("single_protein.pdb"),
                              m)
        IMP.atom.show_molecular_hierarchy(mh)
        mhc= IMP.atom.clone(mh)
        nb= IMP.atom.get_internal_bonds(mh).size()
        nnb= IMP.atom.get_internal_bonds(mhc).size()
        self.assertEqual(nb, nnb)
        IMP.atom.show_molecular_hierarchy(mhc)


if __name__ == '__main__':
    unittest.main()
