import unittest
import RMF
import shutil

class GenericTest(RMF.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_frames(self):
        """Test getting all values for an attribute"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file_frames.rmf"))
        r= f.get_root_node()
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1, 0)
        r.set_value(ik, 2, 1)
        r.set_value(ik, 4, 3)
        av= r.get_all_values(ik)
        self.assertEqual(av, [1,2,RMF.NullInt,4])
if __name__ == '__main__':
    unittest.main()
