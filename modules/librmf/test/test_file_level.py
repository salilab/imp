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
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file.mh"))
        r= f
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1)
        print r.get_value(ik)
        self.assertEqual(r.get_value(ik), 1)
    def test_frames(self):
        """Test creating a simple hierarchy file with frames"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file.mh"))
        r= f
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1, 0)
        self.assertEqual(r.get_value(ik), 1)
        r.set_value(ik, 2, 1)
        self.assertEqual(r.get_value(ik, 1), 2)
        self.assertEqual(r.get_value(ik, 0), 1)
        self.assertEqual(f.get_number_of_frames(ik), 2)
    def test_perturbed_values(self):
        """Test null values"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("test_file.mh"))
        r= f
        print r.get_type()
        sc= f.add_category("sequence")
        ik= f.add_int_key(sc, "ik0", True)
        r.set_value(ik, 1)
        ika= r.get_value_always(ik)
        self.assertEqual(ika, 1)
        ikna= r.get_value_always(ik, 1)
        self.assertEqual(ikna, RMF.NullInt)
        fk= f.add_float_key(sc, "fk0", True)
        r.set_value(fk, 1)
        fka= r.get_value_always(fk)
        self.assertEqual(fka, 1)
        fkna= r.get_value_always(fk, 1)
        self.assertEqual(fkna, RMF.NullFloat)
    def test_backwards_0(self):
        """Test opening pre-category change files"""
        ifn=self.get_input_file_name("backwards.rmf")
        f= RMF.open_rmf_file_read_only(ifn)
        pc= f.get_category("physics")
        pks= f.get_keys(pc)
        self.assert_(len(pks)> 3)
        print pks
        tfn= self.get_tmp_file_name("test_file.mh")
        shutil.copy(ifn, tfn)
        fw= RMF.open_rmf_file(tfn)
        pc= f.get_category("physics")
        pks= f.get_keys(pc)
        self.assert_(len(pks)> 3)
if __name__ == '__main__':
    unittest.main()
