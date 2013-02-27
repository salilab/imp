#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _do_test_type(self, nh, k, v):
        print k, v
        nh.set_value(k, v)
        vo= nh.get_value(k)
        self.assertEqual(vo, v)
    def _do_test_types(self, f):
        nh= f.get_root_node().add_child("testn", RMF.REPRESENTATION)
        cat= f.get_category("mine")
        #g= f.get_hdf5_group()
        #print g
        #ds= g.add_child_float_data_set_2d("ds"+str(pccc))
        #dsi= g.add_child_index_data_set_2d("dsi"+str(pccc))
        lst=[(f.get_int_key, 4),
             (f.get_float_key, 3.1415),
             (f.get_node_id_key, nh.get_id()),
             (f.get_node_ids_key, [nh.get_id()]),
             (f.get_string_key, "there"),
             (f.get_index_key, 3),
             (f.get_ints_key, [3,4,5,6])]
        self.assertEqual(len(lst),len(RMF.get_data_types())-3)
        for i, p in enumerate(lst):
            print p[0]
            k = p[0](cat, "hi"+str(i))
            self._do_test_type(nh, k, p[1])
    def test_data_types(self):
        """Test that the various data types work"""
        print "testing types"
        for suffix in RMF.suffixes:
            name=RMF._get_temporary_file_path("test_data_types."+suffix)
            f= RMF.create_rmf_file(name)
            self._do_test_types(f)

if __name__ == '__main__':
    unittest.main()
