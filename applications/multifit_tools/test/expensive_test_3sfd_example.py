import IMP.test
import os


class Tests(IMP.test.ApplicationTestCase):

    def test_3sfd(self):
        """Test the 3sfd example"""
        cmds = self.read_shell_commands(
            '../../../doc/tutorial/multifit_3sfd.dox')
        d = IMP.test.RunInTempDir()
        for c in cmds[:-1]:
            # Skip the cluster command since it requires a nonstandard
            # Python module (fastcluster)
            if ' cluster ' not in c:
                self.run_shell_command(c)
        # Should have produced 5 models
        for i in range(5):
            os.unlink('model.%d.pdb' % i)

if __name__ == '__main__':
    IMP.test.main()
