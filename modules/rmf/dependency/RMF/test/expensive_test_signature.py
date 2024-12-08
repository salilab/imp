import unittest
import RMF


class Tests(unittest.TestCase):

    def test_1(self):
        """Test signature of older file"""
        try:
            import RMF_HDF5
        except:
            return

        RMF.set_log_level("Off")
        path = RMF._get_test_input_file_path(
            "conformations.imp.old.0fd20c095e58.rmf")
        f = RMF.open_rmf_file_read_only(path)
        sig = RMF.get_signature_string(f)
        print(sig)
        open("conformations.imp.old.0fd20c095e58.signature", "w").write(sig)
        old_sig = open(
            RMF._get_test_input_file_path(
                "conformations.imp.old.0fd20c095e58.signature"),
            "r").read()
        RMF._assert_signatures_equal(sig, old_sig)

if __name__ == '__main__':
    unittest.main()
