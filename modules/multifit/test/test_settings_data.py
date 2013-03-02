import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit

class Tests(IMP.test.TestCase):
    """Class to test correct reading of system data"""
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.VERBOSE)
    def test_read_data(self):
        """test correct reading of settings data"""
        header = IMP.multifit.read_settings(self.get_input_file_name("assembly.input"))
        self.assertEqual(header.get_number_of_component_headers(), 4)
        self.assertEqual(header.get_component_header(2).get_name(), "C")
        self.assertEqual(header.get_assembly_header().get_resolution(), 20.)

if __name__ == '__main__':
    IMP.test.main()
