import IMP.test
import os


class Tests(IMP.test.ApplicationTestCase):

    def test_antibody(self):
        """Test the antibody example"""
        cmds = self.read_shell_commands(
            '../../../doc/manual/kinematics_rrt.dox')
        with IMP.test.temporary_working_directory():
            for c in cmds:
                self.run_shell_command(c)
            expected = ['node_begin.pdb', 'nodes1.pdb']
            for e in expected:
                os.unlink(e)


if __name__ == '__main__':
    IMP.test.main()
