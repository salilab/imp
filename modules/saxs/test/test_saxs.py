import unittest
import IMP
import IMP.test
import IMP.modeller
import IMP.core
import IMP.saxs
import os
import time

class PDBReadTest(IMP.test.TestCase):

    def test_saxs(self):
        """Check reading a pdb with one protein"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.modeller.read_pdb(
                      self.get_input_file_name('single_protein.pdb'), m)
        #IMP.core.show_molecular_hierarchy(mp)

        #! read experimental profile
        exp_saxs_profile = IMP.saxs.SAXSProfile(
        #          self.get_input_file_name('i_single_protein_IMP.txt'))
        #          self.get_input_file_name('i_single_protein_IMP_old.txt'))
                  self.get_input_file_name('i_single_protein_MODELLER.txt'))
        #           self.get_input_file_name('MflB_8586.dat'))
        #exp_saxs_profile.write_SAXS_file('i_single_protein_MODELLER.txt')

        print 'min_q = ' + str(exp_saxs_profile.get_min_q())
        print 'max_q = ' + str(exp_saxs_profile.get_max_q())
        print 'delta_q = ' + str(exp_saxs_profile.get_delta_q())

        #! read form-factors
        f_table = IMP.saxs.FormFactorTable(
                  self.get_input_file_name('formfactors-int_tab_solvation.lib'),
                  exp_saxs_profile.get_min_q(),
                  exp_saxs_profile.get_max_q(),
                  exp_saxs_profile.get_delta_q())
        #f_table.show()

        #! select particles from the model
        particles = IMP.core.get_by_type(mp,
                        IMP.core.MolecularHierarchyDecorator.ATOM)

        #! calculate SAXS profile
        model_saxs_profile = IMP.saxs.SAXSProfile(
                                    exp_saxs_profile.get_min_q(),
                                    exp_saxs_profile.get_max_q(),
                                    exp_saxs_profile.get_delta_q(),
                                    f_table)
        model_saxs_profile.calculate_profile(particles)
        model_saxs_profile.write_SAXS_file('i_single_protein_IMP.txt')

        #! calculate chi-square
        saxs_score = IMP.saxs.SAXSScore(f_table, exp_saxs_profile)
        chi_square = saxs_score.compute_chi_score(model_saxs_profile)
        print 'scaling c = ' + str(saxs_score.get_scaling())
        print 'Chi_square  = ' + str(chi_square)

        t_start = time.clock()
        #! calculate derivatives of chi-square per particle
        chi_derivatives= saxs_score.calculate_chi_derivative(model_saxs_profile,
                                                             particles)
        for i in range(0, chi_derivatives.__len__(), 100):
        #for i in range(0, particles.__len__()):
            temp = 'i=' + str(i) + '\t' + str(chi_derivatives[i][0])
            temp += '\t' + str(chi_derivatives[i][1])
            temp += '\t' + str(chi_derivatives[i][2])
            print temp

        t_end = time.clock()
        print "**Execution Time for SAXS_score = ", t_end - t_start, "seconds"

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
