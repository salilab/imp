import utils
import os
import sys
import unittest
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.test  # noqa: F401   # Test should also be importable


class Tests(unittest.TestCase):
    def test_simple(self):
        """Exercise the ihm.test basic install test"""
        subprocess.check_call([sys.executable,
                               os.path.join(TOPDIR, 'ihm', 'test.py')])


if __name__ == '__main__':
    unittest.main()
