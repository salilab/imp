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
        """Test creating a simple hierarchy file"""
        for suffix in RMF.suffixes:
            f= RMF.create_rmf_file(RMF._get_temporary_file_path("test_file_perturbed."+suffix))
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(ik, 1)
            print r.get_value(ik)
            self.assertEqual(r.get_value(ik), 1)
    def test_frames(self):
        """Test creating a simple hierarchy file with frames"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("test_file_frames."+suffix)
            f= RMF.create_rmf_file(path)
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f0= f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(ik, 1)
            self.assertEqual(r.get_value(ik), 1, 0)
            f1= f0.add_child("1", RMF.FRAME)
            r.set_value(ik, 2)

            del f
            del sc
            del ik
            del r
            del f0
            del f1
            f= RMF.open_rmf_file_read_only(path)
            r= f.get_root_node()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f.set_current_frame(0)
            self.assertEqual(r.get_value(ik), 1, 0)
            f.set_current_frame(1)
            self.assertEqual(r.get_value(ik), 2)
            f.set_current_frame(0)
            self.assertEqual(r.get_value(ik), 1)
            self.assertEqual(f.get_number_of_frames(), 2)
    def test_perturbed_values(self):
        """Test null values int"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("test_filei."+suffix)
            f= RMF.create_rmf_file(path)
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            f0= f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(ik, 1)
            ika= r.get_value_always(ik)
            self.assertEqual(ika, 1)
            f1= f0.add_child("1", RMF.FRAME)
            ikna= r.get_value_always(ik)
            self.assertEqual(ikna, RMF.NullInt)
    def test_perturbed_values(self):
        """Test null values int"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("test_filef."+suffix)
            f= RMF.create_rmf_file(path)
            r= f.get_root_node()
            sc= f.get_category("sequence")
            fk= f.get_float_key(sc, "fk0")
            f0= f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(fk, 1)
            fka= r.get_value_always(fk)
            self.assertEqual(fka, 1)
            f1= f0.add_child("1", RMF.FRAME)
            fkna= r.get_value_always(fk)
            self.assertEqual(fkna, RMF.NullFloat)
if __name__ == '__main__':
    unittest.main()
