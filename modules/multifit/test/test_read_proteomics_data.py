import IMP
import IMP.test
import IMP.multifit
import os

class Tests(IMP.test.TestCase):
    """Tests for proteomics file reader"""

    def test_run(self):
        """Check correct reading of proteomics file"""
        data_file = self.get_input_file_name("proteomics.input")
        prot_data = IMP.multifit.read_proteomics_data(data_file)
        self.assertEqual(prot_data.get_number_of_proteins(),3)
        self.assertEqual(prot_data.get_number_of_interactions(),2)
        self.assertEqual(prot_data.get_start_res(0),1)
        self.assertEqual(prot_data.get_end_res(0),100)
        self.assertEqual(len(prot_data.get_interaction(0)),2)
        self.assertEqual(len(prot_data.get_interaction(1)),3)
        self.assertEqual(prot_data.get_ev_pair(0)[0],0)
        self.assertEqual(prot_data.get_ev_pair(0)[1],1)
        self.assertEqual(prot_data.get_num_allowed_violated_interactions(),1)
        self.assertEqual(prot_data.get_num_allowed_violated_cross_links(), 3)
        self.assertEqual(prot_data.get_num_allowed_violated_ev(),2)

        self.assertEqual(prot_data.get_interaction(0), [0,1])
        self.assertEqual(prot_data.get_interaction(1), [0,1,2])
        self.assertEqual(prot_data.get_protein_name(0), 'A')
        self.assertEqual(prot_data.find('A'), 0)

    def test_bad_protein_line(self):
        """Check incorrectly formatted protein lines in proteomics files"""
        # Not enough fields
        open('test.input', 'w').write('|proteins|\n|A|1|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        # Fields of wrong type
        open('test.input', 'w').write('|proteins|\n|A|x|100|nn|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

    def test_bad_xlink_line(self):
        """Check incorrectly formatted cross link lines in proteomics files"""
        header = '|proteins|\n|interactions|1|\n|residue-xlink|3|\n'
        # Wrong number of fields
        open('test.input', 'w').write(header + '|0|A|30|B|50|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        # Fields of wrong type
        open('test.input', 'w').write(header + '|0|A|x|B|50|10|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

    def test_bad_ev_line(self):
        "Check incorrectly formatted excluded volume lines in proteomics files"
        header = '|proteins|\n|interactions|1|\n|residue-xlink|3|\n' \
                 + '|ev-pairs|2|\n'
        # Wrong number of fields
        open('test.input', 'w').write(header + '|A|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

    def test_bad_interaction_line(self):
        """Check incorrectly formatted interaction lines in proteomics files"""
        header = '|proteins|\n|interactions|1|\n'
        # Wrong number of fields
        open('test.input', 'w').write(header + '|1|A|B|10|authors|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        # Fields of wrong type
        open('test.input', 'w').write(header + '|x|A|B|10|authors|type|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

    def test_bad_interaction_header(self):
        """Check incorrectly formatted interaction header in proteomics files"""
        open('test.input', 'w').write('|proteins|\n|interactions|x|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

    def test_bad_xlink_header(self):
        """Check incorrectly formatted cross link header in proteomics files"""
        open('test.input', 'w').write('|proteins|\n|interactions|1|\n' + \
                                      '|residue-xlink|x|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

    def test_bad_ev_header(self):
        "Check incorrectly formatted excluded volume header in proteomics files"
        open('test.input', 'w').write('|proteins|\n|interactions|1|\n' + \
                                      '|residue-xlink|3|\n|ev-pairs|x|\n')
        self.assertRaises(ValueError, IMP.multifit.read_proteomics_data,
                          'test.input')
        os.unlink('test.input')

if __name__ == '__main__':
    IMP.test.main()
