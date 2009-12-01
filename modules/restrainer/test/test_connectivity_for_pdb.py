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
        IMP.test.TestCase.setUp(self)

        change_to_test_directory()

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('pdb_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('pdb_connectivity_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.to_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        restraint_name = 'connectivity_restraint'
        connectivity_restraint = self.restraint.get_restraint_by_name(restraint_name).imp_restraint
        connectivity_restraint.show()

        protein1_hierarchy = self.representation.find_by_id('Protein1').model_decorator
        IMP.atom.write_pdb (protein1_hierarchy, "test_protein1.pdb")

        root_hierarchy = self.representation.model_decorator
        IMP.atom.write_pdb (root_hierarchy, "test_root.pdb")

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    unittest.main()
