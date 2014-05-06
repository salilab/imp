import IMP.test
import sys
import os
import re


class EM2DDockApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple(self):
        """Simple test of EM2D single PDB score application"""
        p = self.run_application('em2d_single_score',
                                 [self.get_input_file_name('complex-2d.pdb'),
                                  self.get_input_file_name('image_1.pgm'),
                                  self.get_input_file_name('image_2.pgm'),
                                  self.get_input_file_name('image_3.pgm'),
                                  '-s', '2.2', '-n', '200'])  # pixel size, projections number
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        m = re.search('Total\s+score\s+=\s+([\d\.]+)\r?', err)
        self.assertIsNotNone(m, msg="Score output not found in " + str(err))
        self.assertAlmostEqual(float(m.group(1)), 2.84, delta=0.01)

        os.unlink('images.pgm')
        os.unlink('best_projections.pgm')

    def test_coarse_grain(self):
        """Simple test of EM2D single PDB score application"""
        p = self.run_application('em2d_single_score',
                                 [self.get_input_file_name('complex-2d.pdb'),
                                  self.get_input_file_name('image_1.pgm'),
                                  self.get_input_file_name('image_2.pgm'),
                                  self.get_input_file_name('image_3.pgm'),
                                  '-s', '2.2', '-n', '200', '-c'])  # pixel size, projections number
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        m = re.search('Total\s+score\s+=\s+([\d\.]+)\r?', err)
        self.assertIsNotNone(m, msg="Score output not found in " + str(err))
        self.assertAlmostEqual(float(m.group(1)), 2.83, delta=0.01)

        os.unlink('images.pgm')
        os.unlink('best_projections.pgm')

    def test_simple_score(self):
        """Simple test of EM2D score application"""
        p = self.run_application('em2d_score',
                                 [self.get_input_file_name('2p4e.pdb'),
                                  self.get_input_file_name('model4.pdb'),
                                  self.get_input_file_name('transforms10'),
                                  self.get_input_file_name('image_1.pgm'),
                                  self.get_input_file_name('image_2.pgm'),
                                  self.get_input_file_name('image_3.pgm'),
                                  '-s', '2.2', '-n', '200'])  # pixel size, projections number
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('em2d_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 13)
        os.unlink('em2d_score.res')

if __name__ == '__main__':
    IMP.test.main()
