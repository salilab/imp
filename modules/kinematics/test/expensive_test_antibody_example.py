import IMP.test
import sys
import os
import re


class Tests(IMP.test.ApplicationTestCase):

    def test_antibody(self):
        """Test the antibody example"""
        cmds = self.read_shell_commands(
            '../../../doc/tutorial/kinematics_rrt.dox')
        d = IMP.test.RunInTempDir()
        for c in cmds:
            self.run_shell_command(c)
        expected = ['node_begin.pdb',
                    'nodes1.pdb']
        for e in expected:
            os.unlink(e)

if __name__ == '__main__':
    IMP.test.main()
