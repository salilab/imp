import IMP.test
import sys
import os
import re

class Tests(IMP.test.ApplicationTestCase):

    def test_nup133(self):
        """Test the Nup133 example"""
        cmds = self.read_shell_commands('../nup133.dox')
        d = IMP.test.RunInTempDir()
        # Skip running gnuplot on machines that don't have it
        have_gnuplot = os.path.exists('/usr/bin/gnuplot')
        for c in cmds:
            if have_gnuplot or 'gnuplot' not in c:
                self.run_shell_command(c)
        lines = open('3KFO_23922_merge.dat').readlines()
        self.assertIn('Chi = 2.96', lines[1])
        lines = open('3KFO-fill.B99990005_23922_merge.dat').readlines()
        self.assertIn('Chi = 1.14', lines[1])
        expected = ['3KFO_23922_merge.dat',
                    '3KFO-fill.B99990005_23922_merge.dat', 'fit.plt',
                    'profiles.plt']
        if have_gnuplot:
            expected.append('fit.png')
        for e in expected:
            os.unlink(e)

if __name__ == '__main__':
    IMP.test.main()
