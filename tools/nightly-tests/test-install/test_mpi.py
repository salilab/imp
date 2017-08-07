import unittest
import subprocess

class Tests(unittest.TestCase):

    def test_modules_installed(self):
        """Check MPI-dependent Python modules"""
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
