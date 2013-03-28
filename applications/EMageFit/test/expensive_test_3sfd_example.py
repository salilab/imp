import IMP.test
import os
import sys

class Tests(IMP.test.ApplicationTestCase):

    def make_dummy_hex(self, input_file_dir):
        """Rather than running HEXDOCK, just return the files it would have
           produced. This makes the test run faster, and we don't need to
           install HEXDOCK."""
        os.environ['PATH'] = os.getcwd() + os.pathsep + os.environ['PATH']
        open('hex', 'w').write("""#!/usr/bin/env python
import sys, shutil
for line in sys.stdin.readlines():
    spl = line.rstrip('\\r\\n').split()
    if len(spl) == 2 and spl[0] == 'save_transform':
        shutil.copy('%s/' + spl[1], spl[1])
""" % input_file_dir)
        os.chmod('hex', 0755)

    def test_3sfd(self):
        """Test the 3sfd example"""
        if sys.platform == 'win32':
            self.skipTest("does not work on Windows")
        input_file_dir = self.get_input_file_name('')
        cmds = self.read_shell_commands('../3sfd.dox')
        d = IMP.test.RunInTempDir()
        self.make_dummy_hex(input_file_dir)
        for c in cmds:
            self.run_shell_command(c)
        # Should have produced 10 models
        for i in range(10):
            os.unlink('solution-%03d.pdb' % i)
        self.run_shell_command(cmds[-1])

if __name__ == '__main__':
    IMP.test.main()
