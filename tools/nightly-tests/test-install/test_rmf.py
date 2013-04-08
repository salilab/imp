import RMF
import unittest
import os

class RMFInstallTests(unittest.TestCase):

    def test_create(self):
        """Check creation of RMF file"""
        m = RMF.create_rmf_file("test_py.rmf")
        del m
        os.unlink("test_py.rmf")

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
