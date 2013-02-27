#!/usr/bin/env python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_frames(self):
        """Test getting all values for an attribute"""
        RMF.set_log_level("info")
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("test_file_frames."+suffix)
            print path
            f= RMF.create_rmf_file(path)
            r= f.get_root_node()
            print r.get_type()
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            fr0=f.get_root_frame().add_child("0", RMF.FRAME)
            r.set_value(ik, 1)
            fr1=fr0.add_child("1", RMF.FRAME)
            r.set_value(ik, 2)
            fr2=fr1.add_child("2", RMF.FRAME)
            fr3=fr2.add_child("3", RMF.FRAME)
            r.set_value(ik, 4)
            self.assertEqual(f.get_number_of_frames(), 4)
            del fr1
            del fr0
            del fr2
            del fr3
            del sc
            del ik
            del r
            del f
            print "reopening"
            f= RMF.open_rmf_file_read_only(path)
            self.assertEqual(f.get_number_of_frames(), 4)
            r= f.get_root_node();
            sc= f.get_category("sequence")
            ik= f.get_int_key(sc, "ik0")
            print "getting all"
            av= r.get_all_values(ik)
            self.assertEqual(av, [1,2,RMF.NullInt,4])
    def test_decorators(self):
        """Test getting all values through a decorator"""
        RMF.set_log_level("info")
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("test_file_decorator."+suffix)
            print path
            f= RMF.create_rmf_file(path)
            r= f.get_root_node()
            frc= f.get_root_frame()
            fact= RMF.ScoreFactory(f)
            sn= r.add_child("feature", RMF.FEATURE)
            for frame in range(0,6):
                ofrc=frc
                frc= ofrc.add_child(str(frame), RMF.FRAME)
                self.assertEqual(f.get_number_of_frames(), frame+1)
                if frame in [1,4,5]:
                    d= fact.get(sn)
                    d.set_score(frame)
            f.set_current_frame(RMF.ALL_FRAMES)
            self.assertEqual(f.get_number_of_frames(), 6)
            del r
            del frc
            del ofrc
            del f
            del d
            del sn
            print "reopening"
            f= RMF.open_rmf_file_read_only(path)
            self.assertEqual(f.get_number_of_frames(), 6)
            r= f.get_root_node();
            fact= RMF.ScoreConstFactory(f)
            sn= r.get_children()[0]
            print "get"
            all_d= fact.get(sn)
            print "get all"
            all_values= all_d.get_all_scores()
            print all_values
            self.assertEqual(all_values[1], 1)
            self.assertEqual(all_values[4], 4)
            self.assertEqual(all_values[5], 5)
            f.set_current_frame(RMF.ALL_FRAMES)
            print "checking is"
            self.assertEqual(fact.get_is(sn), False)
            print "done"
if __name__ == '__main__':
    unittest.main()
