import unittest
import RMF
import shutil

class Tests(RMF.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test copying a rmf file to a prmf file"""
        nm= self.get_input_file_name("sink.rmf")
        onm= self.get_tmp_file_name("sink_out.prmf")
        f= RMF.open_rmf_file_read_only(nm)
        try:
            of= RMF.create_rmf_file(onm)
        except:
            self.skipTest("No google protobuf support")
        RMF.copy_structure(f, of)
        self.assert_(RMF.get_equal_structure(f, of, True))
        nf= f.get_number_of_frames()
        for i in range(0, nf):
            RMF.copy_frame(f, of, i, i)

        for i in range(0, nf):
            self.assert_(RMF.get_equal_frame(f, of, i, i, True))
    def test_simple(self):
        """Test copying a simple rmf file to prmf"""
        nm= self.get_input_file_name("simple.rmf")
        onm= self.get_tmp_file_name("simple.prmf")
        f= RMF.open_rmf_file_read_only(nm)
        try:
            of= RMF.create_rmf_file(onm)
        except:
            self.skipTest("No google protobuf support")
        RMF.copy_structure(f, of)
        #RMF.show_hierarchy(of.get_root_node())
        of.flush()
        RMF.show_hierarchy(of.get_root_node())
        self.assert_(RMF.get_equal_structure(f, of, True))
        nf= f.get_number_of_frames()
        for i in range(0, nf):
            RMF.copy_frame(f, of, i, i)
        for i in range(0, nf):
            self.assert_(RMF.get_equal_frame(f, of, i, i, True))

if __name__ == '__main__':
    unittest.main()
