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
        root_hierarchy = self.representation.model_decorator

        ub = IMP.algebra.Vector3D(-50.0,-50.0,-50.0)
        lb = IMP.algebra.Vector3D( 50.0, 50.0, 50.0)
        bb = IMP.algebra.BoundingBox3D(ub, lb)

        for i in xrange (1, 4):
            name = "Protein" + str(i) + "_rigid"
            rbs = self.restraint.get_restraint_by_name (name)

            translation = IMP.algebra.random_vector_in_box(bb)
            rotation = IMP.algebra.random_rotation()
            transformation = IMP.algebra.Transformation3D(rotation, translation)

            for rbd in rbs.rigid_bodies:
                rbd.set_transformation(transformation)

        root_hierarchy = self.representation.model_decorator

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    unittest.main()
