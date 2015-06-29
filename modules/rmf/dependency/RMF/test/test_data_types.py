#!/usr/bin/env python
from __future__ import print_function
import unittest
import RMF


class GenericTest(unittest.TestCase):

    def _make_list(self, f):
        if len(f.get_root_node().get_children()) == 0:
            nh = f.get_root_node().add_child("testn", RMF.REPRESENTATION)
        else:
            nh = f.get_root_node().get_children()[0]
        lst = [(RMF.int_tag, 4),
               (RMF.float_tag, 3.25),
               (RMF.string_tag, "there"),
               (RMF.strings_tag, ["here", "there", "everywhere"]),
               (RMF.vector3_tag, RMF.Vector3(1, 2, 3)),
               (RMF.vector4_tag, RMF.Vector4(1, 2, 3, 4)),
               (RMF.vector3s_tag, [RMF.Vector3(1, 2, 3)]),
               (RMF.ints_tag, [3, 4, 5, 6]), ]
        #      (f.get_vector4_key, RMF.Vector4(1,2,3,4))]
        #self.assertEqual(len(lst), len(RMF.get_data_types()) - 3)
        return lst

    def _test_write(self, name):
        f = RMF.create_rmf_file(name)
        f.add_frame("root", RMF.FRAME)
        lst = self._make_list(f)
        nh = f.get_root_node().get_children()[0]
        cat = f.get_category("mine")
        for i, p in enumerate(lst):
            print(i)
            k = f.get_key(cat, "hi" + str(i), p[0])
            print(k, p[1])
            nh.set_value(k, p[1])

    def _test_read(self, name):
        f = RMF.open_rmf_file_read_only(name)
        f.set_current_frame(RMF.FrameID(0))
        lst = self._make_list(f)
        nh = f.get_root_node().get_children()[0]
        cat = f.get_category("mine")
        for i, p in enumerate(lst):
            k = f.get_key(cat, "hi" + str(i), p[0])
            v = nh.get_value(k)
            print(v, p[1])
            # to handle list comparisons
            t = type(p[1])
            self.assertEqual(p[1], t(v))

    def _do_test_types(self, name):
        print("write")
        self._test_write(name)
        print("read")
        self._test_read(name)

    def test_data_types(self):
        """Test that the various data types work"""
        print("testing types")
        RMF.set_log_level("trace")
        for suffix in RMF.suffixes:
            name = RMF._get_temporary_file_path("test_data_types." + suffix)
            #f= RMF.create_rmf_file(name)
            self._do_test_types(name)

if __name__ == '__main__':
    unittest.main()
