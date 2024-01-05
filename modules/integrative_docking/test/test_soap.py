import IMP.test
import sys
import os
import re


class SOAPDockingApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple_single_structure_score(self):
        """Simple test of single complex SOAP score application"""

        p = self.run_application('soap_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb')])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        with open('soap_score.res', 'r') as fh:
            lines = fh.readlines()
        last_line = lines[-1]
        words = last_line.split('|')
        self.assertAlmostEqual(float(words[1]), -1541.274, delta=0.01)
        os.unlink('soap_score.res')

    def test_simple_docking_trans_score(self):
        """Simple test of soap score application for transformations"""
        p = self.run_application('soap_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name('transforms-foxs10')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        with open('soap_score.res', 'r') as fin:
            text = fin.read()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 14)
        os.unlink('soap_score.res')

    def test_simple_filenames_score(self):
        """Simple test of filenames SOAP score application"""

        with open("filenames.txt", "w") as text_file:
            text_file.write(self.get_input_file_name('static.pdb'))
            text_file.write(" ")
            text_file.write(self.get_input_file_name('transformed.pdb'))
        p = self.run_application('soap_score',
                                 ['filenames.txt'])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        with open('soap_score.res', 'r') as fh:
            lines = fh.readlines()
        last_line = lines[-1]
        words = last_line.split('|')
        self.assertAlmostEqual(float(words[1]), -1541.274, delta=0.01)
        os.unlink('soap_score.res')
        os.unlink('filenames.txt')

    def test_soap_score_version(self):
        """Test --version of soap_score"""
        p = self.run_application('soap_score', ['--version'])
        out, err = p.communicate()
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('Version:', err)


if __name__ == '__main__':
    IMP.test.main()
