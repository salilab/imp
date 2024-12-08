import unittest
import os
import subprocess


mock_config = os.environ['MOCK_CONFIG']


class Tests(unittest.TestCase):

    def test_modules_installed(self):
        """Check MPI-dependent Python modules"""
        import IMP.mpi  # noqa: F401
        import IMP.spb  # noqa: F401

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
    unittest.main()
