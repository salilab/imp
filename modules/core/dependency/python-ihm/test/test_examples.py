import utils
import os
import unittest
import sys
import subprocess
import pickle

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)

import ihm.reader

def get_example_dir():
    return os.path.join(TOPDIR, "examples")

def get_example_path(fname):
    return os.path.join(get_example_dir(), fname)

class Tests(unittest.TestCase):

    def test_simple_docking_example(self):
        """Test simple-docking example"""
        with utils.temporary_directory() as tmpdir:
            subprocess.check_call([sys.executable,
                                   get_example_path("simple-docking.py")],
                                  cwd=tmpdir)

            # Make sure that a complete output file was produced and that we
            # can read it
            with open(os.path.join(tmpdir, 'output.cif')) as fh:
                contents = fh.readlines()
            self.assertEqual(len(contents), 271)
            with open(os.path.join(tmpdir, 'output.cif')) as fh:
                s, = ihm.reader.read(fh)

    def test_locations_example(self):
        """Test locations example"""
        subprocess.check_call([sys.executable, "locations.py"],
                              cwd=get_example_dir())
        out = get_example_path("output.cif")

        # Make sure that a complete output file was produced and that we
        # can read it
        with open(out) as fh:
            contents = fh.readlines()
        self.assertEqual(len(contents), 66)
        with open(out) as fh:
            s, = ihm.reader.read(fh)
        os.unlink(out)

    def test_ligands_water_example(self):
        """Test ligands_water example"""
        subprocess.check_call([sys.executable, "ligands_water.py"],
                              cwd=get_example_dir())
        out = get_example_path("output.cif")

        # Make sure that a complete output file was produced and that we
        # can read it
        with open(out) as fh:
            contents = fh.readlines()
        self.assertEqual(len(contents), 147)
        with open(out) as fh:
            s, = ihm.reader.read(fh)
        # Make sure that resulting Python objects are picklable
        testpck = 'test-lig-wat.pck'
        with open(testpck, 'wb') as fh:
            pickle.dump(s, fh, protocol=-1)
        with open(testpck, 'rb') as fh:
            s2 = pickle.load(fh)
        os.unlink(out)
        os.unlink(testpck)

    def test_non_standard_residues_example(self):
        """Test non_standard_residues example"""
        subprocess.check_call([sys.executable, "non_standard_residues.py"],
                              cwd=get_example_dir())
        out = get_example_path("output.cif")

        # Make sure that a complete output file was produced and that we
        # can read it
        with open(out) as fh:
            contents = fh.readlines()
        self.assertEqual(len(contents), 77)
        with open(out) as fh:
            s, = ihm.reader.read(fh)
        os.unlink(out)


if __name__ == '__main__':
    unittest.main()
