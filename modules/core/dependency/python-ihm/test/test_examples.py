import utils
import os
import unittest
import sys
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)

def get_example_path(fname):
    return os.path.join(TOPDIR, "examples", fname)

class Tests(unittest.TestCase):

    def test_simple_docking_example(self):
        """Test simple-docking example"""
        with utils.temporary_directory() as tmpdir:
            subprocess.check_call([sys.executable,
                                   get_example_path("simple-docking.py")],
                                  cwd=tmpdir)

            # Make sure that a complete output file was produced
            with open(os.path.join(tmpdir, 'output.cif')) as fh:
                contents = fh.readlines()
            self.assertEqual(len(contents), 227)


if __name__ == '__main__':
    unittest.main()
