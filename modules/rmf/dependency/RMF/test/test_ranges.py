#!/usr/bin/env python
from __future__ import print_function
import unittest
import RMF


class GenericTest(unittest.TestCase):

    def test_open_2x(self):
        """Test frame comments"""
        f = RMF.create_rmf_file(RMF._get_temporary_file_path("frames.rmf"))
        f.add_frame("root", RMF.FRAME)
        cat = f.get_category("cat")
        key = f.get_key(cat, "key", RMF.int_tag)
        n = f.get_root_node().add_child("node", RMF.REPRESENTATION)
        n.set_value(key, 1)
        print("frames", f.get_number_of_frames())
        f.add_frame("next", RMF.FRAME)
        print("frames", f.get_number_of_frames())
        print(f.get_keys(cat))
        print("frames", f.get_number_of_frames())
        for fr in f.get_frames():
            print(fr)
        print("frames", f.get_number_of_frames())
        self.assertEqual(len(f.get_frames()), 2)


if __name__ == '__main__':
    unittest.main()
