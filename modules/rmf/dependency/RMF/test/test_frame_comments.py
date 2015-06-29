#!/usr/bin/env python
from __future__ import print_function
import unittest
import RMF


class GenericTest(unittest.TestCase):

    def _test_one(self, name):
        print(name)
        f = RMF.create_rmf_file(name)
        f0 = f.add_frame("hi", RMF.FRAME)
        self.assertEqual(f.get_name(f0), "hi")
        print("on 1")
        f1 = f.add_frame("there", RMF.FRAME)
        self.assertEqual(f.get_name(f1), "there")

    def test_open_2x(self):
        """Test frame comments"""
        for suffix in ["rmfz"]:
            self._test_one(RMF._get_temporary_file_path("comments." + suffix))

if __name__ == '__main__':
    unittest.main()
