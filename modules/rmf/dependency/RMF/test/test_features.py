#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _create(self, path):
      fh= RMF.create_rmf_file(path)
      rt= fh.get_root_node()
      pf= RMF.ParticleFactory(fh)
      reps=[rt.add_child("rep"+str(i), RMF.REPRESENTATION) for i in range(0,5)]
      for i, r in enumerate(reps):
          pd= pf.get(r)
          pd.set_coordinates([0,i,0])
          pd.set_mass(1)
          pd.set_radius(.5)
      sf= RMF.ScoreFactory(fh)
      fn= rt.add_child("feature", RMF.FEATURE)
      sd= sf.get(fn)
      sd.set_score(10.0)
      sd.set_representation(reps)
    def _test(self, path):
      fh= RMF.open_rmf_file_read_only(path)
      rt= fh.get_root_node()
      ch= rt.get_children()
      fn= ch[-1]
      reps= ch[:-1]
      sf= RMF.ScoreConstFactory(fh)
      sd= sf.get(fn)
      print sd.get_score()
      print reps
      print sd.get_representation()
      self.assert_(sd.get_score()==10)
      self.assert_(sd.get_representation() == reps)
    def test_multiparent(self):
        """Test that feature nodes work right"""
        for suffix in RMF.suffixes:
            path=RMF._get_temporary_file_path("feature."+suffix)
            print "file is", path
            self._create(path)
            self._test(path)

if __name__ == '__main__':
    unittest.main()
