#!/usr/bin/env python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def _do_test(self, suffix):
        """Test creating a simple hierarchy file with multiple keys"""
        f= RMF.create_rmf_file(RMF._get_temporary_file_path("multikey."+suffix))
        f0=f.get_root_frame().add_child("0", RMF.FRAME)
        r= f.get_root_node()
        print r.get_type()
        sc= f.get_category("multikey")
        ikfp= f.get_int_key(sc, "ik0")
        ik= f.get_int_key(sc, "ik0")
        on= r.add_child("oc", RMF.REPRESENTATION)
        f0.add_child("1", RMF.FRAME)
        r.set_value(ikfp, 1)
        on.set_value(ik, 10)
        print r.get_value(ikfp)
        print on.get_value(ik)
    def test_multiparent(self):
        """Test creating a simple hierarchy file with multiple keys"""
        for i in RMF.suffixes:
            self._do_test(i);


if __name__ == '__main__':
    unittest.main()
