from __future__ import print_function
import unittest
import RMF
import shutil


class GenericTest(unittest.TestCase):

    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print(i, g.get_child_name(i), g.get_child_is_group(i))

    def _copy_to(self, suffix):
        nm = RMF._get_test_input_file_path("sink.rmf3")
        onm = RMF._get_temporary_file_path("sink_out." + suffix)
        print(nm, onm)
        f = RMF.open_rmf_file_read_only(nm)
        of = RMF.create_rmf_file(onm)
        RMF.clone_file_info(f, of)
        RMF.clone_hierarchy(f, of)
        self.assert_(RMF.get_equal_structure(f, of))
        RMF.clone_static_frame(f, of)
        self.assert_(RMF.get_equal_static_values(f, of))
        num_frames = f.get_number_of_frames()
        for fr in f.get_frames():
            f.set_current_frame(fr)
            nfid = of.add_frame(
                f.get_name(fr), f.get_type(fr))

            self.assertEqual(nfid, fr)
            RMF.clone_loaded_frame(f, of)
            self.assert_(RMF.get_equal_current_values(f, of))
            RMF.show_hierarchy_with_values(f.get_root_node())
            RMF.show_hierarchy_with_values(of.get_root_node())

            print("number of frames", of.get_number_of_frames(),\
                fr, f.get_number_of_frames())
        self.assert_(RMF.get_equal_static_values(f, of))
        print("deling")
        del of
        print("reopening")
        of = RMF.open_rmf_file_read_only(onm)
        self.assert_(RMF.get_equal_structure(f, of))
        RMF.show_hierarchy_with_values(f.get_root_node())
        RMF.show_hierarchy_with_values(of.get_root_node())

        self.assert_(RMF.get_equal_static_values(f, of))
        for i in range(0, num_frames):
            fid = RMF.FrameID(i)
            print(fid)
            f.set_current_frame(fid)
            of.set_current_frame(fid)
            if suffix != "rmft":
                # going through a text format perturbs values
                self.assert_(RMF.get_equal_current_values(f, of))

    def test_perturbed(self):
        """Test copying an rmf file"""
        for suffix in RMF.suffixes:
            print(suffix)
            self._copy_to(suffix)

if __name__ == '__main__':
    unittest.main()
