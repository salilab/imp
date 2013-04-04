#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def test_invalid(self):
        """Test that invalid float values work in rmft"""
        name=RMF._get_temporary_file_path("test_data_types.rmft")
        print "file is", name
        f = RMF.create_rmf_file(name)
        cat = f.get_category("test")
        k0 = f.get_float_key(cat, "k0")
        k1 = f.get_float_key(cat, "k1")
        n0 = f.get_root_node().add_child("n0", RMF.REPRESENTATION)
        n1 = f.get_root_node().add_child("n1", RMF.REPRESENTATION)
        n0.set_value(k0, 1.0)
        n1.set_value(k1, 2.0)
        del f
        del n0
        del n1
        f = RMF.open_rmf_file_read_only(name)
        cat = f.get_category("test")
        k0 = f.get_float_key(cat, "k0")
        k1 = f.get_float_key(cat, "k1")
        n0 = f.get_root_node().get_children()[0]
        n1 = f.get_root_node().get_children()[1]
        self.assert_(not n1.get_has_value(k0))

if __name__ == '__main__':
    unittest.main()
