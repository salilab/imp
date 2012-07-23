import unittest
import RMF

class GenericTest(RMF.TestCase):
    def _test_one(self, name):
        f= RMF.create_rmf_file(name)
        f.set_frame_name(10, "hi")
        self.assertEqual(f.get_frame_name(1), "")
        self.assertEqual(f.get_frame_name(10), "hi")
    def test_open_2x(self):
        """Test frame comments"""
        self._test_one(self.get_tmp_file_name("comments.rmf"))
        self._test_one(self.get_tmp_file_name("comments.prmf"))

if __name__ == '__main__':
    unittest.main()
