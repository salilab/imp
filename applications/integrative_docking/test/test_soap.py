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
        fin=open('soap_score.res','r')
        lines = fin.readlines()
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
        fin = open('soap_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 14)
        os.unlink('soap_score.res')


    def test_simple_filenames_score(self):
        """Simple test of filenames SOAP score application"""

        text_file = open("filenames.txt", "w")
        text_file.write(self.get_input_file_name('static.pdb'));
        text_file.write(" ");
        text_file.write(self.get_input_file_name('transformed.pdb'));
        text_file.close()
        p = self.run_application('soap_score',
                                 ['filenames.txt'])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        fin=open('soap_score.res','r')
        lines = fin.readlines()
        last_line = lines[-1]
        words = last_line.split('|')
        self.assertAlmostEqual(float(words[1]), -1541.274, delta=0.01)
        os.unlink('soap_score.res')
        os.unlink('filenames.txt')


if __name__ == '__main__':
    IMP.test.main()
