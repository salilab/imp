import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.helper
import unittest
import os
import time

class SingleDominoRunTests(IMP.test.TestCase):
    """Tests for a domino run on a single mapping"""
    # def setup_filenames(self):
    #     self.jt_fn=self.get_input_file_name("1z5s.jt")
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.settings = IMP.multifit.read_settings(self.get_input_file_name("assembly.input"))
        self.data = IMP.multifit.DataContainer(self.settings)
    def test_run(self):
        """Check that the correct answer is returned"""
        anchor2comp = IMP.ParticlePairs()
        for i in range(4):
            anchor2comp.append(IMP.ParticlePair(self.data.get_density_anchor_point(i),
                                                self.data.get_component(i).get_particle()))
        single_run=IMP.multifit.SingleDominoRun(self.data)
        aa = single_run.run(anchor2comp,900.)
        for i in range(5):
            print aa.show_optimum_configuration(i)
        #for i in range(5):
        #    aa.set_configuration(i)
        #    print self.data.get_model().evaluate(None)
        print "===="
if __name__ == '__main__':
    unittest.main()
