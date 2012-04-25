import unittest
import RMF

class GenericTest(RMF.TestCase):
    def test_aliases(self):
        """Test that aliases can be used and resolve"""
        fh= RMF.create_rmf_file(self.get_tmp_file_name("alias.rmf"))
        rh= fh.get_root_node()
        nh= rh.add_child("hi", RMF.REPRESENTATION)
        RMF.add_child_alias(nh, rh)
        ch= RMF.get_children_resolving_aliases(nh)
        self.assertEqual(len(ch), 1)
        self.assertEqual(ch[0], rh)

if __name__ == '__main__':
    unittest.main()
