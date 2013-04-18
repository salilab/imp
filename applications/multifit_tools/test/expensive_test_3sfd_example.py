import IMP.test
import os

class Tests(IMP.test.ApplicationTestCase):

    def test_3sfd(self):
        """Test the 3sfd example"""
        input_file_dir = self.get_input_file_name('')
        cmds = self.read_shell_commands('../3sfd.dox')
        d = IMP.test.RunInTempDir()
        # Skip last command (clustering) since it requires a nonstandard
        # Python module
        for c in cmds[:-1]:
            self.run_shell_command(c)
        # Should have produced 5 models
        for i in range(5):
            os.unlink('model.%d.pdb' % i)

if __name__ == '__main__':
    IMP.test.main()
