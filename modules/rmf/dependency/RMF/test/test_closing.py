#!/usr/bin/env python
import unittest
import RMF
import shutil

class Tests(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test closing an RMF file"""
        for suffix in RMF.suffixes:
            f= RMF.create_rmf_file(RMF._get_temporary_file_path("test_file."+suffix))
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(ik, 1)
            del r
            del f
            del ik
            del sc
            names= RMF.HDF5.get_open_handle_names()
            print names
            self.assertEqual(len(names), 0)
    def test_perturbed_2(self):
        """Test reopening an RMF file"""
        for suffix in RMF.suffixes:
            name=RMF._get_temporary_file_path("test_file2."+suffix)
            print name
            f= RMF.create_rmf_file(name)
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(ik, 1)
            del f
            del r

            f= RMF.open_rmf_file_read_only(name)
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f.set_current_frame(0)
            self.assertEqual(r.get_value(ik), 1)


if __name__ == '__main__':
    unittest.main()
