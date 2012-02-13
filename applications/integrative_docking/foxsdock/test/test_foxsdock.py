import IMP.test
import sys
import os
import re

class FoXSDockApplicationTest(IMP.test.ApplicationTestCase):
    def test_simple(self):
        """Simple test of SAXS profile application"""
        print self.get_input_file_name('static.pdb')
        print self.get_input_file_name('transformed.pdb')
        print self.get_input_file_name('transforms10')
        print self.get_input_file_name('iq.dat')
        p = self.run_application('saxs_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name('transforms10'),
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
        os.unlink('saxs_score.res')

if __name__ == '__main__':
    IMP.test.main()
