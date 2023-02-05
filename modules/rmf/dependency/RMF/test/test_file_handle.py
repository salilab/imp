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
        self.assertRaises(IOError, fh.set_current_frame, RMF.FrameID(0))
        self.assertRaises(IOError, fh.get_path)
        self.assertRaises(IOError, fh.get_current_frame)
        self.assertRaises(IOError, fh.get_type, RMF.FrameID(0))
        self.assertRaises(IOError, fh.get_name, RMF.FrameID(0))
        self.assertRaises(IOError, fh.get_children, RMF.FrameID(0))
        self.assertRaises(IOError, fh.get_parents, RMF.FrameID(0))
        self.assertRaises(IOError, fh.get_number_of_frames)
        self.assertRaises(IOError, fh.get_number_of_nodes)
        self.assertRaises(IOError, fh.get_file_type)
        self.assertRaises(IOError, fh.get_root_node)

        fh = RMF.FileHandle()
        self.assertTrue(fh.get_is_closed())
        self.assertRaises(IOError, fh.get_root_node)
        self.assertRaises(IOError, fh.add_frame, "f0")
        self.assertRaises(IOError, fh.add_frame, "f1", RMF.FrameID(0))
        self.assertRaises(IOError, fh.set_description, "foo")
        self.assertRaises(IOError, fh.set_producer, "foo")
        self.assertRaises(IOError, fh.flush)


if __name__ == '__main__':
    unittest.main()
