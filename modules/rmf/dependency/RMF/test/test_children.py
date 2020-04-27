from __future__ import print_function
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


if __name__ == '__main__':
    unittest.main()
