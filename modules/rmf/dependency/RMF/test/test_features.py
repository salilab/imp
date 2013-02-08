#!/usr/bin/python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _create(self, path):
      fh= RMF.create_rmf_file(path)
      rt= fh.get_root_node()
      reps=[rt.add_child("rep"+str(i), RMF.REPRESENTATION) for i in range(0,5)]
      sf= RMF.ScoreFactory(fh)
      fn= rt.add_child("feature", RMF.FEATURE)
      sd= sf.get(fn)
      sd.set_representation(reps)
      fh.get_current_frame().add_child("zero", RMF.FRAME)
      sd.set_score(10.0)
    def _test(self, path, frame):
      fh= RMF.open_rmf_file_read_only(path)
      fh.set_current_frame(frame)
      rt= fh.get_root_node()
      ch= rt.get_children()
      fn= ch[-1]
      reps= ch[:-1]
      sf= RMF.ScoreConstFactory(fh)
      sd= sf.get(fn)
      print reps
      print sd.get_representation()
      self.assert_(sd.get_representation() == reps)
      fh.set_current_frame(0)
      print sd.get_score()
      self.assert_(sd.get_score()==10)
    def test_multiparent(self):
        """Test that feature nodes work right"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("alias2."+suffix)
            print path
            self._create(path)
            self._test(path, RMF.ALL_FRAMES)
            self._test(path, 0)

if __name__ == '__main__':
    unittest.main()
