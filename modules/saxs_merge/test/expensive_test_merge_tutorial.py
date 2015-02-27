import IMP.test
import os

class Tests(IMP.test.ApplicationTestCase):

    def test_merge(self):
        """Test the simple merge example"""
        cmds = self.read_shell_commands(
            '../../../doc/tutorial/saxs_merge.dox')
        d = IMP.test.RunInTempDir()
        for c in cmds:
            self.run_shell_command(c)
        # Should have produced mean_ and data_ files
        for prefix in ('mean', 'data'):
            os.unlink('%s_25043_01C_S059_0_01.sub' % prefix)
            os.unlink('%s_25043_01D_S061_0_01.sub' % prefix)
        os.unlink('data_merged.dat')
        os.unlink('mean_merged.dat')
        os.unlink('summary.txt')

if __name__ == '__main__':
    IMP.test.main()
