import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.restrainer
import os
import time

class ReadProteomicsTests(IMP.test.TestCase):
    """Tests for a domino run on a single mapping"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)
        self.data_file = self.get_input_file_name("proteomics.input")
    def test_run(self):
        """Check correct reading of proteomics file"""
        self.prot_data = IMP.multifit.read_proteomics_data(self.data_file)
        self.assertEqual(self.prot_data.get_number_of_proteins(),3)
        self.assertEqual(self.prot_data.get_number_of_interactions(),2)
        self.assertEqual(self.prot_data.get_start_res(0),1)
        self.assertEqual(self.prot_data.get_end_res(0),100)
        self.assertEqual(len(self.prot_data.get_interaction(0)),2)
        self.assertEqual(len(self.prot_data.get_interaction(1)),3)
        print self.prot_data.get_interaction(0)
        print self.prot_data.get_interaction(1)
        for ind in self.prot_data.get_interaction(1):
            print self.prot_data.get_protein_name(ind)
            print self.prot_data.find(self.prot_data.get_protein_name(ind))
        self.prot_data.show()
if __name__ == '__main__':
    IMP.test.main()
