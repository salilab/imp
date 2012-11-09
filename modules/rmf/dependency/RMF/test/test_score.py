#!/usr/bin/python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def test_multiparent(self):
        """Test that score decorators"""
        path=RMF._get_temporary_file_path("alias.rmf")
        print path
        fh= RMF.create_rmf_file(path)
        rh= fh.get_root_node()
        nh= rh.add_child("hi", RMF.REPRESENTATION)
        fact= RMF.ScoreFactory(fh)
        score= rh.add_child("score", RMF.FEATURE)
        dec= fact.get(score)
        dec.set_score(2.0)
        fh.set_current_frame(0)
        dec.set_representation([nh])
        back= dec.get_representation()
        print back
        self.assertEqual([nh], back)
        print "frame"
        fh.set_current_frame(1)
        back= dec.get_representation()
        print back
        self.assertEqual([nh], back)

if __name__ == '__main__':
    unittest.main()
