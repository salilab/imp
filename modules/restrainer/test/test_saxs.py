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

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('saxs_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('saxs_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.to_model()
        self.restraint.add_to_representation(self.representation)


    def test_saxs_restraint(self):
        """Check saxs restraint"""

        restraint_name = 'saxs_restraint'
        r = self.restraint.get_restraint_by_name(restraint_name)

        if r:
            self.assertRaises(IMP.UsageException, r.imp_restraint.evaluate, False)
            #score = r.imp_restraint.evaluate(False)
            #print 'initial score = ' + str(score)

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    unittest.main()
