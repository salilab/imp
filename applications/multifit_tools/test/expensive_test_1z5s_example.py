import IMP.test
import os

class Tests(IMP.test.ApplicationTestCase):

    def test_1z5s(self):
        """Test the 1z5s example"""
        input_file_dir = self.get_input_file_name('')
        cmds = self.read_shell_commands('../1z5s.dox')
        d = IMP.test.RunInTempDir()
        for c in cmds:
            self.run_shell_command(c)
        # Should have produced 5 models
        for i in range(5):
            os.unlink('1z5s.mdl.%d.pdb' % i)

if __name__ == '__main__':
    IMP.test.main()
