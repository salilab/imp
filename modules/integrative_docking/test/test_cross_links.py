import IMP.test
import sys
import os
import re


class CrossLinksApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple_interface(self):
        """Simple test of interface cross links generation application"""
        print self.get_input_file_name('static.pdb')
        print self.get_input_file_name('transformed.pdb')
        p = self.run_application('interface_cross_links',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'), '20'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('cross_links.dat', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 4)
        os.unlink('cross_links.dat')
        os.unlink('cxms2.dat')
        os.unlink('cxms_all.dat')

    def test_simple_single_structure_score(self):
        """Simple test of interface cross links single structure score application"""
        destination = open('complex.pdb', 'w')
        file1 = open(self.get_input_file_name('static.pdb'), 'r')
        file2 = open(self.get_input_file_name('transformed.pdb'), 'rb')
        destination.write(file1.read())
        destination.write(file2.read())
        destination.close()
        p = self.run_application('cross_links_single_score',
                                 ['complex.pdb',
                                  self.get_input_file_name('cxms.dat')])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        m = re.search('Score\s+=\s+([\d\.]+)\r?', err)
        self.assertIsNotNone(m, msg="Score output not found in " + str(err))
        self.assertAlmostEqual(float(m.group(1)), 0.267, delta=0.01)

        os.unlink('complex.pdb')

    def test_simple_score(self):
        """Simple test of cross linking score application"""
        p = self.run_application('cross_links_score',
                                 [self.get_input_file_name('static.pdb'),
                                  self.get_input_file_name('transformed.pdb'),
                                  self.get_input_file_name(
                                      'transforms-foxs10'),
                                  self.get_input_file_name('cxms.dat')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('cxms_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 15)
        os.unlink('cxms_score.res')

if __name__ == '__main__':
    IMP.test.main()
