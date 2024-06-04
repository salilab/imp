import RMF
import unittest
import utils

class Tests(unittest.TestCase):
    def test_children(self):
        """Test handling of node children"""
        name = RMF._get_temporary_file_path("children.rmf")
        f = RMF.create_rmf_file(name)

        rn = f.get_root_node()

        ch = rn.add_child("ch", RMF.REPRESENTATION)

        chb = rn.get_children()[0]
        self.assertIsInstance(ch, RMF.NodeHandle)
        self.assertIsInstance(chb, RMF.NodeHandle)
        self.assertEqual(chb.get_name(), "ch")

        ch2 = rn.add_child("ch2", RMF.REPRESENTATION)
        ch3 = rn.add_child("ch3", RMF.REPRESENTATION)

        self.assertEqual([n.get_name() for n in rn.get_children()],
                         ['ch', 'ch2', 'ch3'])

        rch = rn.replace_child(ch2, "rch", RMF.REPRESENTATION)

        # rch should replace ch2 in root's children
        self.assertEqual([n.get_name() for n in rn.get_children()],
                         ['ch', 'rch', 'ch3'])

        # rch should now be the parent of ch2
        self.assertEqual([n.get_name() for n in rch.get_children()],
                         ['ch2'])
        self.assertEqual(len(ch.get_children()), 0)
        self.assertEqual(len(ch2.get_children()), 0)

        # Failure if no parent-child relationship
        self.assertRaises(IOError, rn.replace_child, ch2, "rch2",
                          RMF.REPRESENTATION)

    def test_children_order(self):
        """Test handling of children out of order"""
        def _make_rmf(name):
            f = RMF.create_rmf_file(name)
            rn = f.get_root_node()
            ch = rn.add_child("ch", RMF.REPRESENTATION)
            ch2 = rn.add_child("ch2", RMF.REPRESENTATION)
            ch3 = rn.add_child("ch3", RMF.REPRESENTATION)
            # rch's index is greater than that of its child ch2, which
            # previously broke rmf_slice
            rch = rn.replace_child(ch2, "rch", RMF.REPRESENTATION)
            f.add_frame("f0")
        name = RMF._get_temporary_file_path("children.rmf")
        name2 = RMF._get_temporary_file_path("children2.rmf")

        _make_rmf(name)

        inr = RMF.open_rmf_file_read_only(name)
        outr = RMF.create_rmf_file(name2)
        RMF.clone_file_info(inr, outr)
        RMF.clone_hierarchy(inr, outr)
        RMF.clone_static_frame(inr, outr)
        inr.set_current_frame(RMF.FrameID(0))
        outr.add_frame("f0")
        RMF.clone_loaded_frame(inr, outr)


if __name__ == '__main__':
    unittest.main()
