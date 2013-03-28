import IMP.test
import IMP.cnmultifit
import sys
import os
import re

class Tests(IMP.test.ApplicationTestCase):

    def test_groel(self):
        """Test the GroEL example"""
        cmds = self.read_shell_commands('../groel.dox')
        d = IMP.test.RunInTempDir()
        for c in cmds:
            self.run_shell_command(c)
        r = re.compile('rmsd:\s+([\d\.]+)')
        rmsds = open('rmsd.output').readlines()
        expected_files = ['1oel_A.pdb.ms', 'multifit.output']
        for i in range(5):
            expected_files.append('asmb.model.%d.pdb' % i)
            expected_files.append('test.%03d.pdb' % i)
        for e in expected_files:
            os.unlink(e)
        del d
        rmsds = [float(r.search(x).group(1)) for x in rmsds]
        # The best scoring model should be close to native
        self.assertLess(rmsds[0], 4.0)

if __name__ == '__main__':
    IMP.test.main()
