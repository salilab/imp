import unittest
import RMF
import shutil


class Tests(unittest.TestCase):

    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print(i, g.get_child_name(i), g.get_child_is_group(i))
    """Test the python code"""

    def test_perturbed(self):
        """Test creating a simple hierarchy file"""
        for suffix in RMF.suffixes:
            f = RMF.create_rmf_file(
                RMF._get_temporary_file_path("test_file_perturbed." + suffix))
            self.assertFalse(f.get_is_closed())
            r = f.get_root_node()
            print(r.get_type())
            sc = f.get_category("sequence")
            ik = f.get_key(sc, "ik0", RMF.int_tag)
            f.add_frame("0", RMF.FRAME)
            r.set_value(ik, 1)
            print(r.get_value(ik))
            self.assertEqual(r.get_value(ik), 1)

    def test_frames(self):
        """Test creating a simple hierarchy file with frames"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("test_file_frames." + suffix)
            f = RMF.create_rmf_file(path)
            f.add_frame("root", RMF.FRAME)
            r = f.get_root_node()
            print(r.get_type())
            sc = f.get_category("sequence")
            ik = f.get_key(sc, "ik0", RMF.int_tag)
            r.set_value(ik, 1)
            self.assertEqual(r.get_value(ik), 1, 0)
            f.add_frame("1", RMF.FRAME)
            r.set_value(ik, 2)

            del f
            del sc
            del ik
            del r
            f = RMF.open_rmf_file_read_only(path)
            r = f.get_root_node()
            sc = f.get_category("sequence")
            ik = f.get_key(sc, "ik0", RMF.int_tag)
            f.set_current_frame(RMF.FrameID(0))
            self.assertEqual(r.get_value(ik), 1, 0)
            f.set_current_frame(RMF.FrameID(1))
            RMF.show_hierarchy_with_values(f.get_root_node())
            self.assertEqual(r.get_value(ik), 2)
            f.set_current_frame(RMF.FrameID(0))
            self.assertEqual(r.get_value(ik), 1)
            self.assertEqual(f.get_number_of_frames(), 2)

    def test_perturbed_values_0(self):
        """Test null values int"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("test_filei." + suffix)
            f = RMF.create_rmf_file(path)
            r = f.get_root_node()
            print(r.get_type())
            sc = f.get_category("sequence")
            ik = f.get_key(sc, "ik0", RMF.int_tag)
            f.add_frame("0", RMF.FRAME)
            r.set_frame_value(ik, 1)
            ika = r.get_value(ik)
            self.assertEqual(ika, 1)
            f1 = f.add_frame("1", RMF.FRAME)
            ikna = r.get_value(ik)
            self.assertEqual(ikna, None)

    def test_perturbed_values(self):
        """Test null values int"""
        for suffix in RMF.suffixes:
            RMF.set_log_level("trace")
            path = RMF._get_temporary_file_path("test_filef." + suffix)
            f = RMF.create_rmf_file(path)
            r = f.get_root_node()
            sc = f.get_category("sequence")
            fk = f.get_key(sc, "fk0", RMF.float_tag)
            f0 = f.add_frame("0", RMF.FRAME)
            r.set_frame_value(fk, 1)
            fka = r.get_value(fk)
            self.assertEqual(fka, 1)
            RMF.show_hierarchy_with_values(r)
            f.add_frame("1", RMF.FRAME)
            RMF.show_hierarchy_with_values(r)
            fkna = r.get_value(fk)
            self.assertEqual(fkna, None)

    def test_base_frames(self):
        """Test initialization of frames"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("test_base_frames." + suffix)
            f = RMF.create_rmf_file(path)
            self.assertEqual(f.get_number_of_frames(), 0)
            self.assertEqual(f.get_current_frame(), RMF.FrameID())
            f.add_frame("hi", RMF.FRAME)
            self.assertEqual(f.get_number_of_frames(), 1)
            self.assertEqual(f.get_current_frame(), RMF.FrameID(0))
            del f
            f = RMF.open_rmf_file_read_only(path)
            self.assertEqual(f.get_number_of_frames(), 1)
            self.assertEqual(f.get_current_frame(), RMF.FrameID())
            f.set_current_frame(RMF.FrameID(0))

    def test_close(self):
        """Test explicit close of file handle"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("test_close." + suffix)
            f = RMF.create_rmf_file(path)
            self.assertEqual(f.get_number_of_frames(), 0)
            f.add_frame("hi", RMF.FRAME)
            self.assertEqual(f.get_number_of_frames(), 1)
            f.close()
            self.assertTrue(f.get_is_closed())
            self.assertRaises(IOError, f.get_number_of_frames)
            f2 = RMF.open_rmf_file_read_only(path)
            self.assertEqual(f2.get_number_of_frames(), 1)

    def test_context_manager(self):
        """Test file handle context manager support"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path(
                "test_context_manager." + suffix)
            with RMF.create_rmf_file(path) as f:
                self.assertEqual(f.get_number_of_frames(), 0)
                f.add_frame("hi", RMF.FRAME)
                self.assertEqual(f.get_number_of_frames(), 1)
            self.assertTrue(f.get_is_closed())
            self.assertRaises(IOError, f.get_number_of_frames)
            with RMF.open_rmf_file_read_only(path) as f2:
                self.assertEqual(f2.get_number_of_frames(), 1)
            self.assertTrue(f2.get_is_closed())


if __name__ == '__main__':
    unittest.main()
