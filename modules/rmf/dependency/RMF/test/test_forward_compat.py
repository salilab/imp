import RMF
import unittest

class Tests(unittest.TestCase):

    def test_new_node_type(self):
        """Make sure we can read RMF files with node types that are new to us"""
        # This file was created by adding
        # const NodeType BIGTYPE(99, "big type");
        # to src/enums.cpp, rebuilding RMF, then creating a simple file with
        # fh = RMF.create_rmf_file("test.rmf3")
        # fh.add_frame('zero', RMF.FRAME)
        # rt = fh.get_root_node()
        # c0 = rt.add_child("c0", RMF.BIGTYPE)
        fname = RMF._get_test_input_file_path("new-node-type.rmf3")
        fh = RMF.open_rmf_file_read_only(fname)
        rt = fh.get_root_node()
        c0, = rt.get_children()
        # Unknown node types should be mapped to the invalid type
        self.assertEqual(c0.get_type(), RMF.INVALID_NODE_TYPE)

if __name__ == '__main__':
    unittest.main()
