import IMP
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    def setUp(self):
        """Place all data from XML files into the IMP Model"""

        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('pdb_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('pdb_connectivity_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.get_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        """Check get_rigid_body_by_id and get_all_rigid_bodies methods"""

        rbd = self.restraint.get_rigid_body_by_id ("Protein1")
        rbd.show()

        rbs = self.restraint.get_all_rigid_bodies()
        rbs[0].show()
        self.assertEqual(len(rbs), 6)

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
