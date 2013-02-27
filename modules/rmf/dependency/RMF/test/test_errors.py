#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def test_data_types(self):
        """Test that opening a non-existent file causes an exception"""
        self.assertRaises(IOError, RMF.open_rmf_file, "no such file")
    def test_open_2x(self):
        """Test that opening a file 2x works"""
        f0= RMF.HDF5.open_file(RMF._get_test_input_file_path("backwards.rmf"))
        RMF.HDF5.set_show_errors(True)
        f1= RMF.HDF5.open_file(RMF._get_test_input_file_path("backwards.rmf"))

if __name__ == '__main__':
    unittest.main()
