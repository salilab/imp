#!/usr/bin/env python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    def _copy_to(self, last_frame, suffix):
        nm= RMF._get_test_input_file_path("sink.rmf")
        onm= RMF._get_temporary_file_path("sink_out."+suffix)
        print nm, onm
        f= RMF.open_rmf_file_read_only(nm)
        of= RMF.create_rmf_file(onm)
        RMF.copy_structure(f, of)
        fr= of.get_root_frame()
        for i in range(-1, last_frame+1):
            f.set_current_frame(i)
            print fr.get_id(), f.get_current_frame().get_id()
            RMF.copy_frame(f, of)
            fr= fr.add_child(str(i), RMF.FRAME)
        print "deling"
        del fr
        del of
        print "reopening"
        of= RMF.open_rmf_file_read_only(onm)
        self.assert_(RMF.get_equal_structure(f, of, True))
        for i in range(-1, last_frame+1):
            print i
            f.set_current_frame(i)
            of.set_current_frame(i)
            if suffix != "rmft":
                # going through a text format perturbs values
                self.assert_(RMF.get_equal_frame(f, of, True))
    def test_perturbed(self):
        """Test copying an rmf file"""
        for suffix in RMF.suffixes:
            print suffix
            self._copy_to(-1, suffix)
            self._copy_to(1, suffix)

if __name__ == '__main__':
    unittest.main()
