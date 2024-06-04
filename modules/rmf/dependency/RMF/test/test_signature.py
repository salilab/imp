import unittest
import RMF

class Tests(unittest.TestCase):

    def test_signature(self):
        """Test get_signature_string()"""
        RMF.set_log_level("Off")
        path = RMF._get_test_input_file_path("simple-new.rmf")
        f = RMF.open_rmf_file_read_only(path)
        sig = RMF.get_signature_string(f)
        with open("simple.rmf.signature", "w") as fh:
            fh.write(sig)
        with open(RMF._get_test_input_file_path("simple.rmf.signature")) as fh:
            old_sig = fh.read()
        RMF._assert_signatures_equal(sig, old_sig)

if __name__ == '__main__':
    unittest.main()
