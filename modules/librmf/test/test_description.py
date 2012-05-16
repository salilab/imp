import unittest
import RMF
import shutil

class GenericTest(RMF.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test the description of an rmf file"""
        fn=self.get_tmp_file_name("test_file_perturbed.rmf")
        f= RMF.create_rmf_file(fn)
        descr= "hi there\n"
        self.assertEqual(f.get_description(), "")
        f.set_description(descr)
        del f
        f= RMF.open_rmf_file(fn)
        odescr= f.get_description()
        self.assertEqual(descr, odescr)
if __name__ == '__main__':
    unittest.main()
