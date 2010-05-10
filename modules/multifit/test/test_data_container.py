import unittest
import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.helper


class SettingsDataTest(IMP.test.TestCase):
    """Class to test correct reading of system data"""
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
    def test_read_data(self):
        """test correct reading of settings data"""
        data_path = os.path.dirname(self.get_input_file_name("assembly.input"))+"/"
        header = IMP.multifit.read_settings(self.get_input_file_name("assembly.input"),data_path)
        data = IMP.multifit.DataContainer(header)
        self.assertEqual(data.get_number_of_components(),4)
        self.assert_(data.get_density_map().get_header().get_resolution()==20.)
        self.assertEqual(data.get_junction_tree().get_number_of_nodes(),2)
if __name__ == '__main__':
    unittest.main()
