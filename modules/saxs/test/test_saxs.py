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

        #read experimental profile
        exp_saxs_profile = IMP.saxs.SAXSProfile(
        #          self.get_input_file_name('i_single_protein_IMP.txt'))
        #          self.get_input_file_name('i_single_protein_IMP_old.txt'))
                  self.get_input_file_name('i_single_protein_MODELLER.txt'))
        #exp_saxs_profile.write_SAXS_file('i_single_protein_MODELLER.txt')

        print 'min_s = ' + str(exp_saxs_profile.get_min_s())
        print 'max_s = ' + str(exp_saxs_profile.get_max_s())
        print 'delta_s = ' + str(exp_saxs_profile.get_delta_s())

        # read form factors
        f_table = IMP.saxs.FormFactorTable(
                  self.get_input_file_name('formfactors-int_tab_solvation.lib'),
                  exp_saxs_profile.get_min_s(),
                  exp_saxs_profile.get_max_s(),
                  exp_saxs_profile.get_delta_s())
        #f_table.show()

        #calculate SAXS profile
        particles = IMP.core.get_by_type(mp,
                        IMP.core.MolecularHierarchyDecorator.ATOM)

        model_saxs_profile = IMP.saxs.SAXSProfile(
                                    exp_saxs_profile.get_min_s(),
                                    exp_saxs_profile.get_max_s(),
                                    exp_saxs_profile.get_delta_s(),
                                    f_table)

        model_saxs_profile.calculate_profile(particles)
        model_saxs_profile.write_SAXS_file('i_single_protein_IMP.txt')

        saxs_score = IMP.saxs.SAXSScore(f_table,
                                        exp_saxs_profile,
                                        model_saxs_profile,
                                        particles)
        chi_square = saxs_score.compute_chi_score(model_saxs_profile)
        print 'scaling c = ' + str(saxs_score.get_scaling())
        print 'Chi_square  = ' + str(chi_square)

        chi_derivatives= saxs_score.calculate_chi_derivative(model_saxs_profile,
                                                             particles)
        for i in range(0, chi_derivatives.__len__(), 100):
        #for i in range(0, particles.__len__()):
            temp = 'i=' + str(i) + '\t' + str(chi_derivatives[i][0])
            temp += '\t' + str(chi_derivatives[i][1])
            temp += '\t' + str(chi_derivatives[i][2])
            print temp

        # Clean up outputs
        for f in ('i_single_protein_IMP.txt', 'fitfile.dat'):
            os.unlink(f)
"""
        saxsdata = IMP.saxs.SaxsData(m, mp)
        saxsdata.initialize(0.009, 0.325, 100,
            505, 15, 'heav',
            self.get_input_file_name('formfactors-int_tab_solvation.lib'),
            'uniform', 0.0, 0.5, 0.0,
            'real', False)

        saxsdata.saxs_intensity('i_s_single_protein.txt', False)
        saxsdata.saxs_read('i_s_single_protein.txt')
        chi_sq = saxsdata.saxs_chifun(True)
        print 'Score native: ' + str(chi_sq)
"""
"""
        saxsdata.ini_saxs(atmsel=atmsel, s_min= 0.009, s_max=0.325, maxs=100,
            nmesh=505, natomtyp=15, represtyp='heav',
            filename='$(LIB)/formfactors-int_tab_solvation.lib',
            wswitch = 'uniform', s_low=0.0, s_hi=0.5, s_hybrid=0.0,
            spaceflag= 'real', use_lookup=False)
"""
if __name__ == '__main__':
    unittest.main()
