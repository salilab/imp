import IMP
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('sphere_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('sphere_diameter_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.get_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        """Check diameter restraint for sphere"""

        restraint_name = 'diameter_restraint'
        r = self.restraint.get_restraint_by_name(restraint_name)

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
