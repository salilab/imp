import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import unittest
import os
import time

class ReadSettingsTests(IMP.test.TestCase):
    """Tests for a domino run on a single mapping"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)
        self.data_path = os.path.dirname(self.get_input_file_name("assembly.input"))+"/"
    def _test_non_existing_input_file(self):
        """Check the code can handle a non existing input file"""
        settings = IMP.multifit.read_settings("wrong_input",self.data_path)
    def test_valid_input_file(self):
        """Check that the correct answer is returned when reading setting file"""
        settings = IMP.multifit.read_settings(self.get_input_file_name("assembly.input"),self.data_path)
        self.assertEqual(settings.get_assembly_header().get_resolution(),20)

if __name__ == '__main__':
    unittest.main()
