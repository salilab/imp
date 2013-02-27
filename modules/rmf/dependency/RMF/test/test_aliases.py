#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def test_multiparent(self):
        """Test that nodes with multiple parents can be used and resolve"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("alias2."+suffix)
            print path
            fh= RMF.create_rmf_file(path)
            rh= fh.get_root_node()
            nh= rh.add_child("hi", RMF.REPRESENTATION)
            nh.add_child(rh)
            ch= nh.get_children()
            self.assertEqual(len(ch), 1)
            print ch
            self.assertEqual(ch[0], rh)

    def test_aliases(self):
        """Test that aliases can be used and resolve"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("alias."+suffix)
            print path
            fh= RMF.create_rmf_file(path)
            af= RMF.AliasFactory(fh)
            rh= fh.get_root_node()
            nh= rh.add_child("hi", RMF.REPRESENTATION)
            RMF.add_child_alias(af, nh, rh)
            ch= nh.get_children()
            self.assertEqual(len(ch), 1)
            print ch
            self.assertEqual(af.get(ch[0]).get_aliased(), rh)


if __name__ == '__main__':
    unittest.main()
