import utils
import os
import sys
import unittest
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)


MAKE_MMCIF = os.path.join(TOPDIR, 'util', 'make-mmcif.py')


class Tests(unittest.TestCase):
    @unittest.skipIf(sys.version_info[0] < 3, "make-mmcif.py needs Python 3")
    def test_simple(self):
        """Simple test of make-mmcif utility script"""
        incif = utils.get_input_file_name(TOPDIR, 'struct_only.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        os.unlink('output.cif')


if __name__ == '__main__':
    unittest.main()
