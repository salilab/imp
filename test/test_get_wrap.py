#!/usr/bin/env python
import unittest
import RMF


class Tests(unittest.TestCase):

    def test_perturbed(self):
        """Test get wrapping"""
        name = RMF._get_temporary_file_path("get_wrap.rmf3")
        fh = RMF.create_rmf_file(name)
        rt = fh.get_root_node()
        cat = fh.get_category("testcat")
        key = fh.get_key(cat, "testkey", RMF.int_tag)
        fh.add_frame("frame")
        self.assertEqual(rt.get_value(key), None)
        self.assertEqual(rt.get_static_value(key), None)
        self.assertEqual(rt.get_frame_value(key), None)


if __name__ == '__main__':
    unittest.main()
