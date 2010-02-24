import unittest
import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class ProteinFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def correaltion_test(self,data):
        for fn,res in data:
            scene = IMP.em.read_map(self.get_input_file_name(fn), self.mrw)
            scene.get_header_writable().set_resolution(res)
            r = IMP.em.FitRestraint(self.particles,scene,
                       IMP.core.XYZR.get_default_radius_key(),
                       IMP.atom.Mass.get_mass_key(),1)
            self.imp_model.add_restraint(r)
            score = self.imp_model.evaluate(False)
            print "EM score (1-CC) = "+str(score), " filename:",fn," res:", res
            self.assert_(score < 0.05, "the correlation score is not correct")
            self.imp_model.remove_restraint(r)

    def test_compare_fit_score_to_imp_generated_maps(self):
        data = [["1z5s_5.imp.mrc",5],
                ["1z5s_10.imp.mrc",10],
                ["1z5s_15.imp.mrc",15],
                ["1z5s_20.imp.mrc",20]]
        self.correaltion_test(data)


    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.mrw=IMP.em.MRCReaderWriter()
        self.imp_model = IMP.Model()
        name=self.get_input_file_name("1z5s.pdb")
        print name
        self.mh = IMP.atom.read_pdb(name,
                                    self.imp_model,IMP.atom.NonWaterPDBSelector())
        IMP.atom.add_radii(self.mh)
        self.particles = IMP.Particles(IMP.core.get_leaves(self.mh))
if __name__ == '__main__':
    unittest.main()
