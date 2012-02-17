import IMP.test
import sys
import os
import re

class EM3DDockApplicationTest(IMP.test.ApplicationTestCase):
    def test_simple(self):
        """Simple test of EM3D single PDB score application"""
        print self.get_input_file_name('complex.pdb')
        print self.get_input_file_name('complex.mrc')
        p = self.run_application('em3d_single_score',
                                 [self.get_input_file_name('complex.pdb'),
                                  self.get_input_file_name('complex.mrc')])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        m = re.search('Best\s+score\s+=\s+([\d\.]+)\r?', err)
        self.assertNotEqual(m, None, msg="Score output not found in " + str(err))
        self.assertAlmostEqual(float(m.group(1)), 3842, delta=1.0)

        os.unlink('em_fit.res')
        os.unlink('fit.pdb')


    def test_simple_score(self):
        """Simple test of EM3D score application"""
        print self.get_input_file_name('2p4e.pdb')
        print self.get_input_file_name('fab.pdb')
        print self.get_input_file_name('transforms10')
        print self.get_input_file_name('complex.mrc')
        p = self.run_application('em3d_score',
                                 [self.get_input_file_name('2p4e.pdb'),
                                  self.get_input_file_name('fab.pdb'),
                                  self.get_input_file_name('complex.mrc'),
                                  self.get_input_file_name('transforms10')])

        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)

        # count the number of lines in output file
        fin = open('em3d_score.res', 'r')
        text = fin.read()
        fin.close()
        number_of_lines = text.count('\n')
        self.assertEqual(number_of_lines, 13)
        os.unlink('em3d_score.res')

if __name__ == '__main__':
    IMP.test.main()
