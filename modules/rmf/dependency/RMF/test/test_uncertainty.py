import sys
import RMF
import unittest
import os
import utils

class Tests(unittest.TestCase):

    def test_scale(self):
        """Test the Provenance decorator"""
        for suffix in RMF.suffixes:
            fname = RMF._get_temporary_file_path("scale." + suffix)
            self._create(fname)
            self._read(fname)

    def _create(self, fname):
        rmf = RMF.create_rmf_file(fname)
        rmf.add_frame('zero', RMF.FRAME)
        rt = rmf.get_root_node()

        scalef = RMF.ScaleFactory(rmf)
        c1 = rt.add_child("c1", RMF.REPRESENTATION)
        scale = scalef.get(c1)
        scale.set_scale(1.0)
        scale.set_upper(10.0)
        scale.set_lower(0.0)
        self._check_scale_node(rmf, c1)

    def _read(self, fname):
        rmf = RMF.open_rmf_file_read_only(fname)
        rt = rmf.get_root_node()
        c1, = rt.get_children()
        self._check_scale_node(rmf, c1)

    def _check_scale_node(self, rmf, node):
        scalef = RMF.ScaleFactory(rmf)
        self.assertTrue(scalef.get_is(node))
        scale = scalef.get(node)
        self.assertAlmostEqual(scale.get_scale(), 1.0, delta=1e-6)
        self.assertAlmostEqual(scale.get_upper(), 10.0, delta=1e-6)
        self.assertAlmostEqual(scale.get_lower(), 0.0, delta=1e-6)


if __name__ == '__main__':
    unittest.main()
