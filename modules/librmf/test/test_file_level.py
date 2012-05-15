import unittest
import RMF
import shutil

class GenericTest(RMF.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test creating a simple hierarchy file"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file_perturbed.rmf"))
        r= f.get_root_node()
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1, 0)
        print r.get_value(ik, 0)
        self.assertEqual(r.get_value(ik, 0), 1)
    def test_frames(self):
        """Test creating a simple hierarchy file with frames"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file_frames.rmf"))
        r= f.get_root_node()
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1, 0)
        self.assertEqual(r.get_value(ik, 0), 1, 0)
        r.set_value(ik, 2, 1)
        self.assertEqual(r.get_value(ik, 1), 2)
        self.assertEqual(r.get_value(ik, 0), 1)
        self.assertEqual(f.get_number_of_frames(), 2)
    def test_perturbed_values(self):
        """Test null values"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file.rmf"))
        r= f.get_root_node()
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1, 0)
        ika= r.get_value_always(ik)
        self.assertEqual(ika, 1)
        ikna= r.get_value_always(ik, 1)
        self.assertEqual(ikna, RMF.NullInt)
        fk= f.add_float_key(sc, "fk0", True)
        r.set_value(fk, 1, 0)
        fka= r.get_value_always(fk)
        self.assertEqual(fka, 1)
        fkna= r.get_value_always(fk, 1)
        self.assertEqual(fkna, RMF.NullFloat)
        RMF.show_hierarchy(r)
        RMF.show_hierarchy_with_decorators(r)
if __name__ == '__main__':
    unittest.main()
