import IMP
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('pdb_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('pdb_connectivity_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.get_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        """Check get_restraint_by_name, get_weight, and set_weight for connectivity restraint for pdb"""

        restraint_name = 'connectivity_restraint_1'
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

            translation = IMP.algebra.get_random_vector_in(bb)
            rotation = IMP.algebra.get_random_rotation_3d()
            transformation = IMP.algebra.Transformation3D(rotation, translation)

            for rbd in rbs.rigid_bodies:
                rbd.set_reference_frame(IMP.algebra.ReferenceFrame3D(transformation))

        root_hierarchy = self.representation.model_decorator

        evr = self.restraint.get_restraint_by_name('excluded_volume_restraint')
        self.assertAlmostEqual(evr.get_weight(), 1.0, delta=1e-4)
        evr = self.restraint.get_restraint_by_name('connectivity_restraint_1')
        self.assertAlmostEqual(evr.get_weight(), 100.0, delta=1e-4)
        evr = self.restraint.get_restraint_by_name('connectivity_restraint_2')
        self.assertAlmostEqual(evr.get_weight(), 1.5, delta=1e-4)
        #evr.set_weight(0.8)
        #self.assertInTolerance (evr.get_weight(), 0.8, 1e-4)

        restraint_set = self.restraint.get_restraint_set_by_name("pulldown_restraint_set")
        self.assertAlmostEqual(restraint_set.get_weight(), 100.0, delta=1e-4)

        self.restraint.show_all_restraints()

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
