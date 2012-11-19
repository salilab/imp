#!/usr/bin/python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _test_one(self, name):
        f= RMF.create_rmf_file(name)
        f.set_current_frame(0)
        print "on 0"
        f.set_frame_name("hi")
        self.assertEqual(f.get_frame_name(), "hi")
        print "on 1"
        f.set_current_frame(1)
        self.assertEqual(f.get_frame_name(), "")
    def test_open_2x(self):
        """Test frame comments"""
        for suffix in RMF.suffixes:
            self._test_one(RMF._get_temporary_file_path("comments."+suffix))

if __name__ == '__main__':
    unittest.main()
