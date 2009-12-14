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

class RestraintTest(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.VERBOSE)
        IMP.test.TestCase.setUp(self)

        change_to_test_directory()

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('em_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('em_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.to_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        r = self.restraint.get_restraint_by_name('em_restraint')
        dmap_header = r.dmap_header

        score = self.Model.evaluate(False)
        print "EM score (1-CC) = "+str(score)
        self.assert_(score < 0.05, "the correlation score is not correct")

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    unittest.main()
