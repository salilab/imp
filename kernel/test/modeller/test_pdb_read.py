import unittest
import IMP
import IMP.test
import IMP.pdb

class PDBReadTest(IMP.test.TestCase):
    #def test_open_error(self):
    #    """Check exception thrown on nonexistant file"""
    #    m=IMP.Model()
    #    self.assertRaises(IMP.IOError,  read_pdb, 'fake_file.pdb', m)
    def test_hierarchy(self):
        """Check reading a pdb with one protein"""
        i_num_res_type= IMP.ResidueType.get_number_unique()
        i_num_atom_type= IMP.AtomType.get_number_unique()
        m = IMP.Model()
        mp= IMP.pdb.read_pdb('modeller/single_protein.pdb', m)
        #mp= IMP.MolecularHierarchyDecorator.cast(p)
        #mp.show()
        #IMP.show_molecular_hierarchy(mp)
        mp.validate()
        hc= IMP.HierarchyCounter()
        IMP.depth_first_traversal(mp, hc)
        f_num_res_type= IMP.ResidueType.get_number_unique()
        f_num_atom_type= IMP.AtomType.get_number_unique()
        mpp= mp.get_parent()
        self.assertEqual(mpp, IMP.MolecularHierarchyDecorator(),
                         "Should not have a parent")
        mpc= mp.get_child(0)
        self.assertEqual(mpc.get_parent(), mp,
                         "Should not have a parent")
        #print str(hc.get_count())
        self.assertEqual(i_num_res_type, f_num_res_type, "too many residue types")
        self.assertEqual(i_num_atom_type, f_num_atom_type, "too many atom types")
        self.assertEqual(1377, hc.get_count(),
                         "Wrong number of particles created")
        rd= IMP.molecular_hierarchy_get_residue(mp, 29)
        self.assertEqual(rd.get_index(), 29);

    def test_bonds(self):
        """Check that the file loader produces bonds"""
        m = IMP.Model()
        mp= IMP.pdb.read_pdb('modeller/single_protein.pdb', m)
        #mp= IMP.MolecularHierarchyDecorator.cast(p)
        all_atoms= IMP.molecular_hierarchy_get_by_type(mp,
                             IMP.MolecularHierarchyDecorator.ATOM);
        self.assertEqual(1221, all_atoms.size(),
                         "Wrong number of atoms found in protein")

    def test_dna(self):
        """Check reading a dna with one chain"""
        m = IMP.Model()
        mp= IMP.pdb.read_pdb('modeller/single_dna.pdb', m)
        print "done reading"
        #IMP.show_molecular_hierarchy(mp)
        #mp= IMP.MolecularHierarchyDecorator.cast(p)
        #mp.show()
        #IMP.show_molecular_hierarchy(mp)
        mp.validate()
        hc= IMP.HierarchyCounter()
        IMP.depth_first_traversal(mp, hc)
        mpp= mp.get_parent()
        self.assertEqual(mpp, IMP.MolecularHierarchyDecorator(),
                         "Should not have a parent")
        mpc= mp.get_child(0)
        self.assertEqual(mpc.get_parent(), mp,
                         "Should not have a parent")
        print str(hc.get_count())
        self.assertEqual(3160, hc.get_count(),
                         "Wrong number of particles created")
        #rd= IMP.get_residue(mp, 29)
        #self.assertEqual(rd.get_index(), 29);

if __name__ == '__main__':
    unittest.main()
