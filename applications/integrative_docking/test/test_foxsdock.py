import IMP.test
import sys
import os
import re


class FoXSDockApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple(self):
        """Simple test of SAXS profile application"""
        p = self.run_application('saxs_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name(
                                      'transforms-foxs10'),
                                  self.get_input_file_name('iq.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('saxs_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 15)
        fin=open('saxs_score.res','r')
        lines = fin.readlines()
        last_line = lines[-2]
        words = last_line.split('|')
        self.assertAlmostEqual(float(words[1]), 0.69, delta=0.1)
        os.unlink('saxs_score.res')


    def test_simple2(self):
        """Simple test 2 of SAXS profile application"""
        p = self.run_application('saxs_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name(
                                      'transforms-foxs10'),
                                  self.get_input_file_name('iq.dat'), '-t'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('saxs_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 15)
        fin=open('saxs_score.res','r')
        lines = fin.readlines()
        last_line = lines[-2]
        words = last_line.split('|')
        self.assertAlmostEqual(float(words[1]), 0.495, delta=0.1)
        os.unlink('saxs_score.res')



if __name__ == '__main__':
    IMP.test.main()
