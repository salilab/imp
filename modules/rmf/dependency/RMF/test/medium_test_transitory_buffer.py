#!/usr/bin/env python
from __future__ import print_function
import unittest
import RMF
import shutil


class GenericTest(unittest.TestCase):

    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print(i, g.get_child_name(i), g.get_child_is_group(i))
    """Test the python code"""

    def test_backwards_0(self):
        """Test opening briefly off buffer works"""
        ifn = RMF._get_test_input_file_path("transitory_buffer.buf")
        buf = RMF.read_buffer(ifn)
        f = RMF.open_rmf_buffer_read_only(buf)
        RMF.show_hierarchy_with_values(f.get_root_node())
if __name__ == '__main__':
    unittest.main()
