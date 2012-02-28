import unittest
import RMF

class GenericTest(RMF.TestCase):
    def test_data_types(self):
        """Test that associations work from python"""
        fh =RMF.create_rmf_file(self.get_tmp_file_name("assoc.rmf"))
        rh =fh.get_root_node()
        rh.set_association(fh)
        rh2= fh.get_node_from_association(fh)
        self.assertEqual(rh2, rh)

if __name__ == '__main__':
    unittest.main()
