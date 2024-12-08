import unittest
import RMF

class Tests(unittest.TestCase):

    def test_validate_ok(self):
        """Test validate() on an OK file"""
        path = RMF._get_test_input_file_path("simple-new.rmf")
        f = RMF.open_rmf_file_read_only(path)
        RMF.validate(f)

if __name__ == '__main__':
    unittest.main()
