import IMP
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('sphere_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('sphere_distance_restraint.xml'))

        DisplayParser = IMP.restrainer.XMLDisplay(self.get_input_file_name('sphere_display.xml'))
        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()
        self.display = DisplayParser.run()

        self.Model = self.representation.get_model()
        self.restraint.add_to_representation(self.representation)
        #self.log = self.display.create_log(self.representation, 'sphere_log_%03d.py')
        #self.log.write('sphere_initial.py')


    def test_show(self):
        """Check distance restraint for sphere"""

        opt = IMP.core.ConjugateGradients()
        opt.set_model(self.Model)
        #opt.add_optimizer_state(self.log)

        opt.optimize(10)
        #self.log.write('sphere_optimized.py')

        restraint_name = 'distance_restraint_1'
        r = self.restraint.get_restraint_by_name(restraint_name)
        self.assertLess(r.imp_restraint.evaluate(False), 10,
                        "unexpected distance score")


        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
