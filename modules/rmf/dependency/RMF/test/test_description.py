#!/usr/bin/env python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test the description of an rmf file"""
        for suffix in RMF.suffixes:
            fn=RMF._get_temporary_file_path("test_file_perturbed."+suffix)
            f= RMF.create_rmf_file(fn)
            descr= "hi there\n"
            self.assertEqual(f.get_description(), "")
            f.set_description(descr)
            del f
            f= RMF.open_rmf_file_read_only(fn)
            odescr= f.get_description()
            self.assertEqual(descr, odescr)
if __name__ == '__main__':
    unittest.main()
