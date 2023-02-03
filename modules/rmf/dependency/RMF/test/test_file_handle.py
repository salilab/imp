import unittest
import RMF


class Tests(unittest.TestCase):

    def test_closed_repr(self):
        """Test show of closed file handle"""
        fh = RMF.FileConstHandle()
        _ = repr(fh)
        fh = RMF.FileHandle()
        _ = repr(fh)

    def test_closed_file_methods(self):
        """Test methods on closed files"""
        fh = RMF.FileConstHandle()
        self.assertTrue(fh.get_is_closed())
        self.assertRaises(OSError, fh.set_current_frame, RMF.FrameID(0))
        self.assertRaises(OSError, fh.get_path)
        self.assertRaises(OSError, fh.get_current_frame)
        self.assertRaises(OSError, fh.get_type, RMF.FrameID(0))
        self.assertRaises(OSError, fh.get_name, RMF.FrameID(0))
        self.assertRaises(OSError, fh.get_children, RMF.FrameID(0))
        self.assertRaises(OSError, fh.get_parents, RMF.FrameID(0))
        self.assertRaises(OSError, fh.get_number_of_frames)
        self.assertRaises(OSError, fh.get_number_of_nodes)
        self.assertRaises(OSError, fh.get_file_type)
        self.assertRaises(OSError, fh.get_root_node)

        fh = RMF.FileHandle()
        self.assertTrue(fh.get_is_closed())
        self.assertRaises(OSError, fh.get_root_node)
        self.assertRaises(OSError, fh.add_frame, "f0")
        self.assertRaises(OSError, fh.add_frame, "f1", RMF.FrameID(0))
        self.assertRaises(OSError, fh.set_description, "foo")
        self.assertRaises(OSError, fh.set_producer, "foo")
        self.assertRaises(OSError, fh.flush)


if __name__ == '__main__':
    unittest.main()
