import unittest
import os

class IHMInstallTests(unittest.TestCase):

    def test_import(self):
        """Check that ihm package was installed"""
        import ihm
        import ihm.dataset
        import ihm.restraint
        import ihm.model
        s = ihm.System()

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
