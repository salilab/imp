#!/usr/bin/python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def test_data_types(self):
        """Test that opening a non-existent file causes an exception"""
        self.assertRaises(IOError, RMF.open_hdf5_file, "no such file")
    def test_open_2x(self):
        """Test that opening a file 2x works"""
        f0= RMF.open_hdf5_file(RMF._get_test_input_file_path("backwards.rmf"))
        RMF.set_show_hdf5_errors(True)
        f1= RMF.open_hdf5_file(RMF._get_test_input_file_path("backwards.rmf"))

if __name__ == '__main__':
    unittest.main()
