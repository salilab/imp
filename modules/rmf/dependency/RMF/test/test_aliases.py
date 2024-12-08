import unittest
import RMF


class Tests(unittest.TestCase):

    def test_multiparent(self):
        """Test that nodes with multiple parents can be used and resolve"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("alias2." + suffix)
            print(path)
            fh = RMF.create_rmf_file(path)
            rh = fh.get_root_node()
            nh = rh.add_child("hi", RMF.REPRESENTATION)
            nh.add_child(rh)
            ch = nh.get_children()
            self.assertEqual(len(ch), 1)
            print(ch)
            self.assertEqual(ch[0], rh)

    def test_aliases(self):
        """Test that aliases can be used and resolve"""
        for suffix in RMF.suffixes:
            path = RMF._get_temporary_file_path("alias." + suffix)
            print(path)
            fh = RMF.create_rmf_file(path)
            print("create factory")
            af = RMF.AliasFactory(fh)
            rh = fh.get_root_node()
            nh = rh.add_child("hi", RMF.REPRESENTATION)
            af.get(nh.add_child("alias", RMF.ALIAS)).set_aliased(rh)
            ch = nh.get_children()
            self.assertEqual(len(ch), 1)
            print(ch)
            print("final check")
            print(af.get(ch[0]).get_aliased())
            self.assertEqual(af.get(ch[0]).get_aliased(), rh)
            print("done")

if __name__ == '__main__':
    unittest.main()
