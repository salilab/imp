import unittest
import sys
import os
import subprocess


mock_config = os.environ['MOCK_CONFIG']


class Tests(unittest.TestCase):

    def test_modules_installed(self):
        """Check MPI-dependent Python modules"""
        # Fedora and RHEL 8 only have a Python 3 MPI module; other systems
        # only have Python 2
        if 'fedora' in mock_config or 'epel-8' in mock_config:
            if sys.version_info[0] == 2:
                self.skipTest("No Python2 support for MPI")
        elif sys.version_info[0] >= 3:
            self.skipTest("No Python3 support for MPI")
        import IMP.mpi
        import IMP.spb

    def test_applications_installed(self):
        """Check install of MPI-dependent binaries"""
        p = subprocess.Popen(['spb_test_score', '--help'],
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        out = p.stdout.read()
        p.stdout.close()
        ret = p.wait()
        self.assertEqual(ret, 0,
                         "spb_test_score exited with %d, %s" % (ret, out))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
