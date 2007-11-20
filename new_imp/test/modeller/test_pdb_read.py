import unittest
import IMP
import IMP.test
import IMP.pdb

class PDBReadTest(IMP.test.IMPTestCase):
    #def test_open_error(self):
    #    """Check exception thrown on nonexistant file"""
    #    m=IMP.Model()
    #    self.assertRaises(IMP.IOError,  read_pdb, 'fake_file.pdb', m)
    def test_hierarchy(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()
        p= IMP.pdb.read_pdb('modeller/single_protein.pdb', m)
        mp= IMP.MolecularHierarchyDecorator.cast(p)
        #mp.show()
        #IMP.show_molecular_hierarchy(mp)
        mp.validate()
        hc= IMP.HierarchyCounter()
        IMP.depth_first_traversal(mp, hc)
        print str(hc.get_count())
        self.assertEqual(1376, hc.get_count(),
                         "Wrong number of particles created")
    def test_bonds(self):
        """Check that the file loader produces bonds"""
        m = IMP.Model()
        p= IMP.pdb.read_pdb('modeller/single_protein.pdb', m)
        mp= IMP.MolecularHierarchyDecorator.cast(p)
        all_atoms= IMP.get_particles(mp,
                                     IMP.MolecularHierarchyDecorator.ATOM);
        self.assertEqual(1221, all_atoms.size(),
                         "Wrong number of atoms found in protein")
        cc= IMP.get_bonded(IMP.BondedDecorator.cast(all_atoms[0]))
        self.assertEqual(cc.size(), all_atoms.size(),
                         "All atoms in protein are not bonded")


if __name__ == '__main__':
    unittest.main()
