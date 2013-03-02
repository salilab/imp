import IMP
import IMP.test
import IMP.multifit

class Tests(IMP.test.TestCase):

    def test_non_existing_input_file(self):
        """Check the code can handle a non existing input file"""
        self.assertRaises(ValueError, IMP.multifit.read_settings, "wrong_input")

    def test_valid_input_file(self):
        "Check that the correct answer is returned when reading setting file"
        settings = IMP.multifit.read_settings(
                                 self.get_input_file_name("assembly.input"))
        settings.set_was_used(True)
        self.assertEqual(settings.get_assembly_header().get_resolution(),20)
        self.assertEqual(settings.get_number_of_component_headers(), 4)

if __name__ == '__main__':
    IMP.test.main()
