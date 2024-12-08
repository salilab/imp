import unittest
import RMF
import utils


class GenericTest(unittest.TestCase):

    def test_invalid(self):
        """Test that invalid float values work in rmft"""
        # not much of a test, no text support at the moment
        name = RMF._get_temporary_file_path("test_data_types.rmf")
        print("file is", name)
        f = RMF.create_rmf_file(name)
        f.add_frame("root", RMF.FRAME)
        cat = f.get_category("test")
        k0 = f.get_key(cat, "k0", RMF.float_tag)
        k1 = f.get_key(cat, "k1", RMF.float_tag)
        n0 = f.get_root_node().add_child("n0", RMF.REPRESENTATION)
        n1 = f.get_root_node().add_child("n1", RMF.REPRESENTATION)
        n0.set_value(k0, 1.0)
        n1.set_value(k1, 2.0)
        del f
        del n0
        del n1
        f = RMF.open_rmf_file_read_only(name)
        f.set_current_frame(RMF.FrameID(0))
        cat = f.get_category("test")
        k0 = f.get_key(cat, "k0", RMF.float_tag)
        k1 = f.get_key(cat, "k1", RMF.float_tag)
        n0 = f.get_root_node().get_children()[0]
        n1 = f.get_root_node().get_children()[1]
        self.assertFalse(n1.get_has_value(k0))

if __name__ == '__main__':
    unittest.main()
