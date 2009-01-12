import unittest
import IMP
import IMP.test
import IMP.modeller
import IMP.core
import IMP.saxs
import os

class PDBReadTest(IMP.test.TestCase):

    def test_saxs(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        mp= IMP.modeller.read_pdb(
                      self.get_input_file_name('single_protein.pdb'), m)
        #IMP.core.show_molecular_hierarchy(mp)

        saxsdata = IMP.saxs.SaxsData(m, mp)
        saxsdata.initialize(0.009, 0.325, 100,
            505, 15, 'heav',
            self.get_input_file_name('formfactors-int_tab_solvation.lib'),
            'uniform', 0.0, 0.5, 0.0,
            'real', False)

        saxsdata.saxs_intensity('i_s_single_protein.txt', False)
        #saxs.saxs_read(filename='i_s.txt')

        # Clean up outputs
        for f in ('i_s_single_protein.txt', 'p_r.txt'):
            os.unlink(f)

"""
        saxsdata.ini_saxs(atmsel=atmsel, s_min= 0.009, s_max=0.325, maxs=100,
            nmesh=505, natomtyp=15, represtyp='heav',
            filename='$(LIB)/formfactors-int_tab_solvation.lib',
            wswitch = 'uniform', s_low=0.0, s_hi=0.5, s_hybrid=0.0,
            spaceflag= 'real', use_lookup=False)
"""
if __name__ == '__main__':
    unittest.main()
