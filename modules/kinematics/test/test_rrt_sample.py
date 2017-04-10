from __future__ import print_function
import IMP.test
import sys
import os

class Tests(IMP.test.ApplicationTestCase):

    def test_simple(self):
        """Simple test of rrt_sample application"""
        p = self.run_application('rrt_sample',
                                 [self.get_input_file_name('three.pdb'),
                                  self.get_input_file_name('hinges.txt'),
                                  '-n', '1'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        self.assertIn('2 joints were constructed', err)
        for out in ('node_begin.pdb', 'nodes1.pdb'):
            os.unlink(out)

    def test_no_dofs(self):
        """Test rrt_sample application with no DOFs"""
        p = self.run_application('rrt_sample',
                                 [self.get_input_file_name('three.pdb'),
                                  self.get_input_file_name('hinges.txt'),
                                  '-c', self.get_input_file_name('connect.txt'),
                                  '-n', '1'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertNotEqual(p.returncode, 0)
        self.assertIn('2 joints were constructed', err)
        self.assertIn('No degrees of freedom to sample!', err)
        os.unlink('node_begin.pdb')

if __name__ == '__main__':
    IMP.test.main()
