import utils
import os
import sys
import unittest
import subprocess

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.reader


MAKE_MMCIF = os.path.join(TOPDIR, 'util', 'make-mmcif.py')


class Tests(unittest.TestCase):
    @unittest.skipIf(sys.version_info[0] < 3, "make-mmcif.py needs Python 3")
    def test_simple(self):
        """Simple test of make-mmcif utility script"""
        incif = utils.get_input_file_name(TOPDIR, 'struct_only.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(s.title,
                         'Architecture of Pol II(G) and molecular mechanism '
                         'of transcription regulation by Gdown1')
        os.unlink('output.cif')

    @unittest.skipIf(sys.version_info[0] < 3, "make-mmcif.py needs Python 3")
    def test_no_title(self):
        """Check that make-mmcif adds missing title"""
        incif = utils.get_input_file_name(TOPDIR, 'no_title.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(s.title, 'Auto-generated system')
        os.unlink('output.cif')

    @unittest.skipIf(sys.version_info[0] < 3, "make-mmcif.py needs Python 3")
    def test_bad_usage(self):
        """Bad usage of make-mmcif utility script"""
        ret = subprocess.call([sys.executable, MAKE_MMCIF])
        self.assertEqual(ret, 1)

    @unittest.skipIf(sys.version_info[0] < 3, "make-mmcif.py needs Python 3")
    def test_mini(self):
        """Check that make-mmcif works given only basic atom info"""
        incif = utils.get_input_file_name(TOPDIR, 'mini.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.state_groups), 1)
        self.assertEqual(len(s.state_groups[0]), 1)
        self.assertEqual(len(s.state_groups[0][0]), 1)
        self.assertEqual(len(s.state_groups[0][0][0]), 1)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.protocol.name, 'modeling')
        self.assertEqual(m.assembly.name, 'Complete assembly')
        chainA, chainB, = m.representation
        for chain in chainA, chainB:
            self.assertIsInstance(chain, ihm.representation.AtomicSegment)
            self.assertFalse(chain.rigid)
        self.assertEqual(s.title, 'Auto-generated system')
        os.unlink('output.cif')

    @unittest.skipIf(sys.version_info[0] < 3, "make-mmcif.py needs Python 3")
    def test_pass_through(self):
        """Check that make-mmcif passes through already-compliant files"""
        incif = utils.get_input_file_name(TOPDIR, 'docking.cif')
        subprocess.check_call([sys.executable, MAKE_MMCIF, incif])
        with open('output.cif') as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.state_groups), 1)
        self.assertEqual(len(s.state_groups[0]), 1)
        self.assertEqual(len(s.state_groups[0][0]), 1)
        self.assertEqual(len(s.state_groups[0][0][0]), 1)
        m = s.state_groups[0][0][0][0]
        self.assertEqual(m.protocol.name, 'Modeling')
        self.assertEqual(m.assembly.name, 'Our complete assembly')
        chainA, chainB, = m.representation
        self.assertIsInstance(chainA, ihm.representation.AtomicSegment)
        self.assertTrue(chainA.rigid)
        self.assertIsInstance(chainB, ihm.representation.FeatureSegment)
        self.assertFalse(chainB.rigid)
        self.assertEqual(s.title, 'Output from simple-docking example')
        os.unlink('output.cif')


if __name__ == '__main__':
    unittest.main()
