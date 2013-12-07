import IMP.test
import sys
import os
import re


class SOAPDockingApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple_single_structure_score(self):
        """Simple test of single complex SOAP score application"""

        p = self.run_application('soap_single_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb')])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('Score\s+=\s+(-[\d\.]+)', err)
        self.assertIsNotNone(m, msg="Score output not found in " + str(err))
        self.assertAlmostEqual(float(m.group(1)), -9007.91, delta=0.01)

    def test_simple_score(self):
        """Simple test of cross linking score application"""
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

if __name__ == '__main__':
    IMP.test.main()
