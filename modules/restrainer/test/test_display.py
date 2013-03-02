import IMP
import IMP.test
import IMP.restrainer
import IMP.display

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('display_representation.xml'))
        DisplayParser = IMP.restrainer.XMLDisplay(self.get_input_file_name('pdb_display.xml'))

        self.representation = RepParser.run()
        self.display = DisplayParser.run()

        self.Model = self.representation.get_model()

        self.log = self.display.create_log(self.representation, 'pdb_log_%03d.py')
        self.log.write('initial_display_in_chimera.py')

    def test_show(self):
        """Check display"""

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
