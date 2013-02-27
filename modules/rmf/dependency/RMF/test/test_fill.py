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
        """Test filling a data set"""
        fn= RMF._get_temporary_file_path("fill.hdf5")
        f= RMF.create_hdf5_file(fn)
        ds= f.add_child_float_data_set_2d("test")
        ds.set_size([1,3])
        ds.set_value([0,0], .5)
        ds.set_value([0,1], 1)
        ds.set_value([0,2], 0)
        ds.set_size([3,4])
if __name__ == '__main__':
    unittest.main()
