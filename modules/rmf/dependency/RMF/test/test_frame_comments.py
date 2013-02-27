#!/usr/bin/env python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _test_one(self, name):
        print name
        f= RMF.create_rmf_file(name)
        f0= f.get_root_frame().add_child("hi", RMF.FRAME)
        self.assertEqual(f0.get_name(), "hi")
        print "on 1"
        f1= f0.add_child("there", RMF.FRAME)
        self.assertEqual(f1.get_name(), "there")
    def test_open_2x(self):
        """Test frame comments"""
        for suffix in RMF.suffixes:
            self._test_one(RMF._get_temporary_file_path("comments."+suffix))

if __name__ == '__main__':
    unittest.main()
