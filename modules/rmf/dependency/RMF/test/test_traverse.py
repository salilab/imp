from __future__ import print_function
import unittest
import RMF

class Tests(unittest.TestCase):

    def test_traverse(self):
        """Test TraverseHelper"""
        path = RMF._get_test_input_file_path("simple-new.rmf")
        f = RMF.open_rmf_file_read_only(path)
        f.set_current_frame(RMF.FrameID(0))
        t = RMF.TraverseHelper(f.get_root_node(), "testmol")
        self.assertEqual(t.get_chain_id(), None)
        self.assertEqual(t.get_residue_index(), None)
        self.assertEqual(t.get_residue_type(), None)
#       self.assertEqual(t.get_molecule_name(), "testmol")
        self.assertEqual(t.get_rgb_color(), None)
        self.assertEqual(t.get_state_index(), 0)
        self.assertEqual(t.get_copy_index(), None)
#       self.assertEqual(t.get_global_coordinates(RMF.Vector3(0,0,0)), None)
        self.assertEqual(len(t.get_children()), 1)

if __name__ == '__main__':
    unittest.main()
