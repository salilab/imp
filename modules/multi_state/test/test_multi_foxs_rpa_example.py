import IMP.test
import sys
import os
import re
import shutil
import IMP.multi_state

class Tests(IMP.test.ApplicationTestCase):

    def test_multi_foxs_rpa(self):
        """Test multi_foxs with RPA example"""
        cmds = ["multi_foxs --help",
                "multi_foxs weighted.dat 1fguA.pdb 1fguB.pdb 1jmc.pdb"]
        rpa = IMP.multi_state.get_example_path('rpa')
        with IMP.test.temporary_working_directory():
            shutil.copytree(rpa, 'rpa')
            os.chdir('rpa')
            for c in cmds:
                self.run_shell_command(c)
            expected = ['cluster_representatives.txt',
                        'multi_state_model_1_1_1.dat',
                        'multi_state_model_1_2_1.dat',
                        'multi_state_model_1_3_1.dat',
                        'ensembles_size_1.txt',
                        'multi_state_model_2_1_1.dat',
                        'multi_state_model_2_2_1.dat',
                        'ensembles_size_2.txt',
                        'multi_state_model_3_1_1.dat',
                        'ensembles_size_3.txt']
            for e in expected:
                os.unlink(e)

if __name__ == '__main__':
    IMP.test.main()
