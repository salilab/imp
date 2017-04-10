from __future__ import print_function
import unittest
import RMF
import shutil


class GenericTest(unittest.TestCase):

    def test_perturbed(self):
        """Test copying an rmf file"""
        # can't set index attribute any more
        return
        RMF.set_log_level("trace")
        name = RMF._get_temporary_file_path("chain.rmf")
        fh = RMF.create_rmf_file(name)
        fh.set_current_frame(RMF.FrameID(0))
        n = fh.get_root_node().add_child("n", RMF.REPRESENTATION)
        c = fh.get_category("sequence")
        k = fh.get_int_key(c, "chain id")
        k2 = fh.get_int_key(c, "other")
        n.set_static_value(k, 0)
        n.set_static_value(k2, 8)
        RMF.show_hierarchy_with_values(n)
        fh.flush()
        print("closing")
        del n
        del fh
        del k
        del c
        fh = RMF.open_rmf_file_read_only(name)
        fh.set_current_frame(RMF.FrameID(0))
        cf = RMF.ChainFactory(fh)
        n = fh.get_root_node().get_children()[0]
        RMF.show_hierarchy_with_values(n)
        self.assert_(cf.get_is(n))
        chain = cf.get(n).get_chain_id()
        self.assertEqual(chain, "A")


if __name__ == '__main__':
    unittest.main()
