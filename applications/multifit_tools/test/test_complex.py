import IMP.test
import os
import sys


class ComplexApplicationTest(IMP.test.ApplicationTestCase):

    def test_simple(self):
        """Simple test of complex_to_anchor_graph application"""
        p = self.run_python_application('complex_to_anchor_graph',
                                        [self.get_input_file_name('mini.pdb'), 'mini.cmm'])
        out, err = p.communicate()
        sys.stderr.write(err)
        self.assertApplicationExitedCleanly(p.returncode, err)
        lines = open('mini.cmm').readlines()
        # Only one chain, so only a single marker and set
        self.assertEqual(len(lines), 2)
        self.assertTrue(lines[0].startswith('<marker_set name="'))
        self.assertTrue(lines[1].startswith('<marker id="0"'))
        self.assertTrue(lines[1].rstrip('\r\n').endswith(' /></marker_set>'))
        os.unlink('mini.cmm')

if __name__ == '__main__':
    IMP.test.main()
