import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.restrainer
import os
import time

class Tests(IMP.test.TestCase):
    """Tests for a domino run on a single mapping"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.TERSE)
        self.data_file = self.get_input_file_name("anchors.input")
    def test_run(self):
        """Check correct reading of anchors data file"""
        self.anchors_data = IMP.multifit.read_anchors_data(self.data_file)
        self.assertEqual(self.anchors_data.get_number_of_points(),3)
        self.assertEqual(self.anchors_data.get_number_of_edges(),2)

if __name__ == '__main__':
    IMP.test.main()
