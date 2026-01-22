import unittest
import RMF


class GenericTest(unittest.TestCase):

    def test_data_types(self):
        """Test that opening a non-existent file causes an exception"""
        self.assertRaises(IOError, RMF.open_rmf_file_read_only, "no such file")

    def test_open_2x(self):
        """Test that opening a file 2x works"""
        try:
            import RMF_HDF5  # noqa: F401
        except ImportError:
            self.skipTest("no support for legacy HDF5 files")
        # Don't use backwards.rmf, but a copy of it, since if we run tests
        # in parallel backwards.rmf might be open at the same time by
        # medium_test_backwards.py, and HDF5 will report an error if we
        # try to have the same file open RO and RW at the same time.
        f0 = RMF.HDF5.open_file(
            RMF._get_test_input_file_path("backwards_copy.rmf"))
        RMF.HDF5.set_show_errors(True)
        f1 = RMF.HDF5.open_file(
            RMF._get_test_input_file_path("backwards_copy.rmf"))


if __name__ == '__main__':
    unittest.main()
