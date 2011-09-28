import unittest
import RMF

class GenericTest(RMF.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test creating a simple hierarchy file"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file.mh"))
        r= f
        print r.get_type()
        ik= f.add_int_key(RMF.Sequence, "ik0", False)
        r.set_value(ik, 1)
        print r.get_value(ik)
        self.assertEqual(r.get_value(ik), 1)

if __name__ == '__main__':
    unittest.main()
