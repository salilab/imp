import IMP.test
import sys
import os
import re


class NMR_RTCApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple_interface(self):
        """Simple test of interface_rtc application"""
        print self.get_input_file_name('static.pdb')
        print self.get_input_file_name('transformed.pdb')
        p = self.run_application('interface_rtc',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('receptor_rare4.txt', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 4)
        os.unlink('receptor_rare3.txt')
        os.unlink('receptor_rare4.txt')
        os.unlink('receptor_ecoli.txt')
        os.unlink('ligand_rare3.txt')
        os.unlink('ligand_rare4.txt')
        os.unlink('ligand_ecoli.txt')

    def test_simple_score(self):
        """Simple test of nmr_rtc_score application"""
        p = self.run_application('nmr_rtc_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name(
                                      'transforms-foxs10'),
                                  self.get_input_file_name('receptor_rare4.txt'), '-'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('nmr_rtc_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 16)
        os.unlink('nmr_rtc_score.res')

if __name__ == '__main__':
    IMP.test.main()
