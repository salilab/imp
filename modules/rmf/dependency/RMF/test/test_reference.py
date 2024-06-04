import RMF
import unittest
import os

class Tests(unittest.TestCase):

    def test_reference(self):
        """Test the Reference decorator"""
        for suffix in RMF.suffixes:
            fname = RMF._get_temporary_file_path("reference." + suffix)
            self._create(fname)
            self._read(fname)

    def _create(self, fname):
        rmf = RMF.create_rmf_file(fname)
        rmf.add_frame('zero', RMF.FRAME)
        rt = rmf.get_root_node()

        rf = RMF.ReferenceFactory(rmf)
        c0 = rt.add_child("c0", RMF.REPRESENTATION)
        c1 = rt.add_child("c1", RMF.REPRESENTATION)
        ref = rf.get(c1)
        ref.set_reference(c0)
        self.assertEqual(ref.get_reference(), c0)
        self.assertFalse(rf.get_is(c0))
        self.assertTrue(rf.get_is(c1))

    def _read(self, fname):
        rmf = RMF.open_rmf_file_read_only(fname)
        rt = rmf.get_root_node()
        c0, c1 = rt.get_children()
        rf = RMF.ReferenceFactory(rmf)
        ref = rf.get(c1)
        self.assertEqual(ref.get_reference(), c0)
        self.assertFalse(rf.get_is(c0))
        self.assertTrue(rf.get_is(c1))

if __name__ == '__main__':
    unittest.main()
