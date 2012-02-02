import unittest
import RMF

class GenericTest(RMF.TestCase):
    def test_data_types(self):
        """Test that opening a non-existent file causes and exception"""
        self.assertRaises(IOError, RMF.open_hdf5_file, "no such file")

if __name__ == '__main__':
    unittest.main()
