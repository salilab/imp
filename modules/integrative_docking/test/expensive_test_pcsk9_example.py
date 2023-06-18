import IMP.test
import os
import sys


class Tests(IMP.test.ApplicationTestCase):

    def make_dummy_patch_dock(self, input_docking):
        """Rather than running PatchDock, just return the files it would have
           produced. This makes the test run faster, and we don't need to
           install PatchDock."""
        os.environ['PATCH_DOCK_HOME'] = os.getcwd()
        with open('buildParams.pl', 'w') as fh:
            fh.write("#!/usr/bin/perl\n# do nothing\n")
        os.chmod('buildParams.pl', 0o755)

        with open('patch_dock.Linux', 'w') as fh:
            fh.write("""#!%s
import sys, shutil
params, outfname = sys.argv[1:]
shutil.copy('%s', outfname)
""" % (sys.executable, input_docking))
        os.chmod('patch_dock.Linux', 0o755)

        with open('interface_cluster.linux', 'w') as fh:
            fh.write("#!" + sys.executable + """
from __future__ import print_function
import sys
receptor, ligand, trans_in, dummy, clustered_out = sys.argv[1:]

fh = open(clustered_out, 'w')
print("receptorPdb (str) " + receptor, file=fh)
print("ligandPdb (str) " + ligand, file=fh)

for n, line in enumerate(open(trans_in)):
    spl = line.rstrip('\\r\\n').split()
    spl[0] = int(spl[0])
    for i in range(1, 8):
        spl[i] = float(spl[i])
    print("%4d | %.4f | %5d | %4d |%.5f %.5f %.5f %.5f %.5f %.5f"
          % tuple(spl[0:2] + [n+1, 1] + spl[2:]), file=fh)
""")
        os.chmod('interface_cluster.linux', 0o755)

    def test_pcsk9(self):
        """Test the pcsk9 example"""
        if sys.platform == 'win32':
            self.skipTest("does not work on Windows")
        cmds = self.read_shell_commands(
            '../../../doc/manual/idock_pcsk9.dox')
        input_docking = self.get_input_file_name('docking.res.pcsk9')
        with IMP.test.temporary_working_directory():
            self.make_dummy_patch_dock(input_docking)
            for c in cmds:
                self.run_shell_command(c)
            # Should have produced 3 solutions
            with open('results_cxms_soap.txt') as fh:
                wc = len(fh.readlines())
            self.assertEqual(wc, 6) # account for 3 header lines

if __name__ == '__main__':
    IMP.test.main()
