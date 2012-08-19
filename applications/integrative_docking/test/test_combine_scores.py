import IMP.test
import sys
import os
import re

class CombineScoresApplicationTest(IMP.test.ApplicationTestCase):
    def test_scores_combination(self):
        """Simple test of score combination"""
        print self.get_input_file_name('saxs_score.res')
        print self.get_input_file_name('em2d_score.res')
        print self.get_input_file_name('em3d_score.res')
        p = self.run_application('combine_scores',
                                 [self.get_input_file_name('saxs_score.res'), '1',
                                  self.get_input_file_name('em2d_score.res'), '1',
                                  self.get_input_file_name('em3d_score.res'), '1'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in the output
        number_of_lines = out.count('\n')
        self.assertEqual(number_of_lines, 41)

    def test_z_score_computation(self):
        """Simple test of zscore combination"""
        print self.get_input_file_name('saxs_score.res')
        p = self.run_application('recompute_zscore',
                                 [self.get_input_file_name('saxs_score.res')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in the output
        number_of_lines = out.count('\n')
        self.assertEqual(number_of_lines, 1001)

if __name__ == '__main__':
    IMP.test.main()
