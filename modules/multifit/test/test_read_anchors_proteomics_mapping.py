import IMP
import os.path
import IMP.test
import IMP.multifit

class Tests(IMP.test.TestCase):

    def test_run(self):
        """Check correct reading of protein anchors mapping file"""
        data_file = self.get_input_file_name("anchors.proteomics.input")
        mapping_file = self.get_input_file_name(
                                        "proteomics.anchors.mapping.txt")
        prot_data = IMP.multifit.read_proteomics_data(data_file)
        mapping = IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                             mapping_file)
        self.assertEqual(os.path.realpath(
                               mapping.get_paths_filename_for_protein("A")),
                         os.path.realpath(
                               self.get_input_file_name('1z5s.A.paths.txt')))
        self.assertEqual(os.path.realpath(mapping.get_anchors_filename()),
                         os.path.realpath(
                            self.get_input_file_name('1z5s_15_14_anchors.txt')))
        self.assertEqual(mapping.get_paths_for_protein("A"), [])

if __name__ == '__main__':
    IMP.test.main()
