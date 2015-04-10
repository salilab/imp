#!/usr/bin/env python
import unittest
import RMF


class Tests(unittest.TestCase):

    def test_0(self):
        """Test that signatures make sense and are stable"""
        try:
            import RMF_HDF5
        except:
            return

        RMF.set_log_level("Off")
        path = RMF._get_test_input_file_path("rep_and_geom.rmf")
        f = RMF.open_rmf_file_read_only(path)
        sig = RMF.get_signature_string(f)
        # print sig
        old_sig = open(
            RMF._get_test_input_file_path(
                "rep_and_geom.signature"),
            "r").read(
        )
        RMF._assert_signatures_equal(sig, old_sig)

    def test_1(self):
        """Test that signatures make sense and are stable"""
        try:
            import RMF_HDF5
        except:
            return

        RMF.set_log_level("Off")
        path = RMF._get_test_input_file_path("final_NPC2007_new.rmf")
        f = RMF.open_rmf_file_read_only(path)
        sig = RMF.get_signature_string(f)
        # print sig
        old_sig = open(
            RMF._get_test_input_file_path(
                "final_NPC2007_new.signature"),
            "r").read(
        )
        RMF._assert_signatures_equal(sig, old_sig)


if __name__ == '__main__':
    unittest.main()
