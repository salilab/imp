import IMP
import IMP.test
import IMP.restrainer

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.base.set_log_level(IMP.base.VERBOSE)
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('em_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('em_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.get_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        """Check EM restraint"""

        r = self.restraint.get_restraint_by_name('em_restraint')
        dmap_header = r.dmap_header

        self.assertAlmostEqual(dmap_header.get_xorigin(), -6.0, delta=1e-4)
        self.assertAlmostEqual(dmap_header.get_yorigin(), -6.0, delta=1e-4)
        self.assertAlmostEqual(dmap_header.get_zorigin(), -6.0, delta=1e-4)
        self.assertAlmostEqual(dmap_header.get_spacing(), 1.0, delta=1e-4)
        self.assertAlmostEqual(dmap_header.get_resolution(), 3.0, delta=1e-4)

        r.imp_restraint.evaluate(False)
        score = self.Model.evaluate(False)
        print "EM score (1-CC) = "+str(score)
        self.assertLess(score, 0.05, "the correlation score is not correct")

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
