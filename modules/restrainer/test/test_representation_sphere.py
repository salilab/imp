import sys
import os
import unittest
import IMP
import IMP.test
import IMP.restrainer

def change_to_test_directory():
    test_directory = os.environ.get('TEST_DIRECTORY')
    if not test_directory:
        test_directory = os.path.dirname(os.path.abspath(sys.argv[0]))
    os.chdir(test_directory)

class RepresentationTest(IMP.test.TestCase):
    def test_representation_parser(self):
        change_to_test_directory()

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('sphere_representation.xml'))
        repr = RepParser.run()

        M = repr.to_model()
        M.show()
        M.evaluate(False)
        self.assertEqual(len(M.get_particles()), 17)

if __name__ == '__main__':
    unittest.main()
