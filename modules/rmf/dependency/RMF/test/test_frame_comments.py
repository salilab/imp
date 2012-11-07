#!/usr/bin/python
import unittest
import RMF

class GenericTest(unittest.TestCase):
    def _test_one(self, name):
        f= RMF.create_rmf_file(name)
        f.set_current_frame(10)
        f.set_frame_name("hi")
        f.set_current_frame(1)
        self.assertEqual(f.get_frame_name(), "")
        f.set_current_frame(10)
        self.assertEqual(f.get_frame_name(), "hi")
    def test_open_2x(self):
        """Test frame comments"""
        self._test_one(RMF._get_temporary_file_path("comments.rmf"))

if __name__ == '__main__':
    unittest.main()
