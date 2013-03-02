import IMP
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.base.set_log_level(IMP.base.VERBOSE)
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('saxs_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('saxs_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.get_model()
        self.restraint.add_to_representation(self.representation)


    def test_saxs_restraint(self):
        """Check saxs restraint"""

        restraint_name = 'saxs_restraint'
        r = self.restraint.get_restraint_by_name(restraint_name)

        score = r.imp_restraint.evaluate(False)
        self.assertAlmostEqual(score, 0.54, delta=1e-2)

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
