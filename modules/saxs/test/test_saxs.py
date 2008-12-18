import unittest
import IMP
import IMP.test
import IMP.modeller
import IMP.core
import IMP.saxs

class PDBReadTest(IMP.test.TestCase):

    def test_saxs(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        #mp = IMP.modeller.read_pdb('single_protein.pdb', m)
        mp= IMP.modeller.read_pdb(self.get_test_file('single_protein.pdb'), m)

        saxsdata = IMP.saxs.SaxsData(m, mp);
        saxsdata.ini_saxs(0.009, 0.325, 100,
            505, 15, 'heav',
            '$(LIB)/formfactors-int_tab_solvation.lib',
            'uniform', 0.0, 0.5, 0.0,
            'real', False)
"""
        saxsdata.ini_saxs(atmsel=atmsel, s_min= 0.009, s_max=0.325, maxs=100,
            nmesh=505, natomtyp=15, represtyp='heav',
            filename='$(LIB)/formfactors-int_tab_solvation.lib',
            wswitch = 'uniform', s_low=0.0, s_hi=0.5, s_hybrid=0.0,
            spaceflag= 'real', use_lookup=False)
"""
"""
        #i_num_res_type= IMP.core.ResidueType.get_number_unique()
        #i_num_atom_type= IMP.core.AtomType.get_number_unique()

        mp.show()
        IMP.core.show_molecular_hierarchy(mp)
        mp.validate()

        ps = IMP.core.molecular_hierarchy_get_by_type(mp, IMP.core.MolecularHierarchyDecorator.ATOM)

        for p in ps:
            d = IMP.core.XYZDecorator.cast(p)
            d.show()
            #d.get_coordinates().show()

        hc= IMP.core.HierarchyCounter()
        IMP.core.depth_first_traversal(mp, hc)
        f_num_res_type= IMP.core.ResidueType.get_number_unique()
        f_num_atom_type= IMP.core.AtomType.get_number_unique()
        mpp= mp.get_parent()
        self.assertEqual(mpp, IMP.core.MolecularHierarchyDecorator(),
                         "Should not have a parent")
        mpc= mp.get_child(0)
        self.assertEqual(mpc.get_parent(), mp,
                         "Should not have a parent")
        #print str(hc.get_count())
        self.assertEqual(i_num_res_type, f_num_res_type, "too many residue types")
        self.assertEqual(i_num_atom_type, f_num_atom_type, "too many atom types")
        self.assertEqual(1377, hc.get_count(),
                         "Wrong number of particles created")
        rd= IMP.core.molecular_hierarchy_get_residue(mp, 29)
        self.assertEqual(rd.get_index(), 29);
        """
if __name__ == '__main__':
    unittest.main()
