import sys
import os
import unittest
import IMP
import IMP.test
import IMP.restrainer

class RestraintTest(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.VERBOSE)
        IMP.test.TestCase.setUp(self)

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('em_representation.xml'))
        RestraintParser = IMP.restrainer.XMLRestraint(self.get_input_file_name('em_restraint.xml'))

        self.representation = RepParser.run()
        self.restraint = RestraintParser.run()

        self.Model = self.representation.to_model()
        self.restraint.add_to_representation(self.representation)


    def test_show(self):
        r = self.restraint.get_restraint_by_name('em_restraint')
        dmap_header = r.dmap_header

        self.assertInTolerance (dmap_header.get_xorigin(), 0.0, 1e-4)
        self.assertInTolerance (dmap_header.get_yorigin(), 0.0, 1e-4)
        self.assertInTolerance (dmap_header.get_zorigin(), 0.0, 1e-4)
        self.assertInTolerance (dmap_header.get_spacing(), 1.0, 1e-4)
        self.assertInTolerance (dmap_header.get_resolution(), 3.0, 1e-4)

        score = self.Model.evaluate(False)
        print "EM score (1-CC) = "+str(score)
        self.assert_(score < 0.05, "the correlation score is not correct")

        self.Model.show()
        self.Model.evaluate(False)

if __name__ == '__main__':
    unittest.main()
