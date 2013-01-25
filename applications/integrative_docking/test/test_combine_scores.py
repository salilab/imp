import IMP.test
import sys
import os
import re

class CombineScoresApplicationTest(IMP.test.ApplicationTestCase):
    def test_usage(self):
        """Test usage of combine_scores"""
        p = self.run_application('combine_scores', [])
        out, err = p.communicate()
        self.assertIn("Usage", err)
        self.assertEqual(p.returncode, 1)

    def test_bad_file(self):
        """Test combine_scores with non-existing file"""
        p = self.run_application('combine_scores', ['not-exist', '1.0'])
        out, err = p.communicate()
        self.assertIn("Can't open file", err)
        self.assertNotEqual(p.returncode, 0)

    def test_mismatch_num_transforms(self):
        """Test combine_scores with mismatched numbers of transforms"""
        lines = ["# |  Score | filt| ZScore | Transformation",
                 "1 |  0.000 |  -  | 0.000 |   -3.0 0.9 -0.4 36.8 -11.2 27.7",
                 "2 |  0.000 |  -  | 0.000 |   -3.0 0.9 -0.4 36.8 -11.2 27.7",
                 "3 |  0.000 |  -  | 0.000 |   -3.0 0.9 -0.4 36.8 -11.2 27.7"]
        open('test1', 'w').write("\n".join(lines) + "\n")
        open('test2', 'w').write("\n".join(lines[:3]) + "\n")
        p = self.run_application('combine_scores',
                                 ['test1', '1.0', 'test2', '1.0'])
        out, err = p.communicate()
        self.assertIn("different number of transformations", err)
        self.assertNotEqual(p.returncode, 0)
        os.unlink('test1')
        os.unlink('test2')

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

    def test_z_score_usage(self):
        """Test usage of recompute_zscores"""
        p = self.run_application('recompute_zscore', [])
        out, err = p.communicate()
        self.assertIn("Usage", err)
        self.assertEqual(p.returncode, 1)

    def test_z_score_bad_file(self):
        """Test recompute_zscore with non-existing file"""
        p = self.run_application('recompute_zscore', ['not-exist'])
        out, err = p.communicate()
        self.assertIn("Can't open file", err)
        self.assertNotEqual(p.returncode, 0)

if __name__ == '__main__':
    IMP.test.main()
