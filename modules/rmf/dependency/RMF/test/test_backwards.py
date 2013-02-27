#!/usr/bin/env python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_backwards_0(self):
        """Test opening pre-category change files"""
        ifn=RMF._get_test_input_file_path("backwards.rmf")
        f= RMF.open_rmf_file_read_only(ifn)
        pc= f.get_category("physics")
        pks= f.get_keys(pc)
        print pks
        self.assert_(len(pks)> 3)
        print pks
        tfn= RMF._get_temporary_file_path("test_fileold.rmf")
        shutil.copy(ifn, tfn)
        fw= RMF.open_rmf_file(tfn)
        pc= f.get_category("physics")
        pks= f.get_keys(pc)
        print pks
        self.assert_(len(pks)> 3)
if __name__ == '__main__':
    unittest.main()
