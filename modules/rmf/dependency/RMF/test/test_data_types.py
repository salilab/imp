#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _make_list(self, f):
      if len(f.get_root_node().get_children()) == 0:
        nh= f.get_root_node().add_child("testn", RMF.REPRESENTATION)
      else:
        nh = f.get_root_node().get_children()[0]
      lst=[(f.get_int_key, 4),
           (f.get_float_key, 3.1415),
           (f.get_node_id_key, nh.get_id()),
           (f.get_node_ids_key, [nh.get_id()]),
           (f.get_string_key, "there"),
           (f.get_index_key, 3),
           (f.get_ints_key, [3,4,5,6])]
      self.assertEqual(len(lst),len(RMF.get_data_types())-3)
      return lst
    def _test_write(self, name):
      f = RMF.create_rmf_file(name)
      lst = self._make_list(f)
      nh = f.get_root_node().get_children()[0]
      cat= f.get_category("mine")
        #g= f.get_hdf5_group()
        #print g
        #ds= g.add_child_float_data_set_2d("ds"+str(pccc))
        #dsi= g.add_child_index_data_set_2d("dsi"+str(pccc))
      for i, p in enumerate(lst):
        k = p[0](cat, "hi"+str(i))
        nh.set_value(k, p[1])
    def _test_read(self, name):
      f = RMF.open_rmf_file_read_only(name)
      lst = self._make_list(f)
      nh = f.get_root_node().get_children()[0]
      cat= f.get_category("mine")
      for i, p in enumerate(lst):
        k = p[0](cat, "hi"+str(i))
        v = nh.get_value(k)
        self.assertEqual(v, p[1])
    def _do_test_types(self, name):
        self._test_write(name)
        self._test_read(name)
    def test_data_types(self):
        """Test that the various data types work"""
        print "testing types"
        for suffix in RMF.suffixes:
            name=RMF._get_temporary_file_path("test_data_types."+suffix)
            #f= RMF.create_rmf_file(name)
            self._do_test_types(name)

if __name__ == '__main__':
    unittest.main()
