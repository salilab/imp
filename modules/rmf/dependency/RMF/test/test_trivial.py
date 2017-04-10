from __future__ import print_function
import unittest
import RMF


class GenericTest(unittest.TestCase):

    def test_aliases(self):
        """Test trivial usage"""
        try:
            import RMF_HDF5
            RMF_HDF5.set_show_errors(True)
        except:
            pass
        for suffix in RMF.suffixes:
            RMF.set_log_level("trace")
            path = RMF._get_temporary_file_path("trivial." + suffix)
            print(path)
            print("create")
            fh = RMF.create_rmf_file(path)
            print("add frame")
            fh.add_frame("root", RMF.FRAME)
            print("keys")
            cat = fh.get_category("MyCat")
            key = fh.get_key(cat, "MyKey", RMF.int_tag)
            print("set")
            fh.get_root_node().set_static_value(key, 1)
            self.assertEqual(fh.get_root_node().get_value(key), 1)
            RMF.show_hierarchy_with_values(fh.get_root_node())
            del fh
            print("open")
            fh = RMF.open_rmf_file_read_only(path)
            print("set frame")
            fh.set_current_frame(RMF.FrameID(0))
            print("keys")
            RMF.show_hierarchy_with_values(fh.get_root_node())
            cat = fh.get_category("MyCat")
            key = fh.get_key(cat, "MyKey", RMF.int_tag)
            print("get value")
            v = fh.get_root_node().get_value(key)
            self.assertEqual(v, 1)

if __name__ == '__main__':
    unittest.main()
