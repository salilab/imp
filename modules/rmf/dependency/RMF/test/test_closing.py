#!/usr/bin/python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test closing an RMF file"""
        f= RMF.create_rmf_file(RMF._get_temporary_file_path("test_file.rmf"))
        r= f.get_root_node()
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        f.set_current_frame(0)
        r.set_value(ik, 1)
        del r
        del f
        del ik
        del sc
        names= RMF.get_open_hdf5_handle_names()
        print names
        self.assertEqual(len(names), 0)

if __name__ == '__main__':
    unittest.main()
