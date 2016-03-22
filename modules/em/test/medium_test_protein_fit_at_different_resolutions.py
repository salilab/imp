from __future__ import print_function
import os
import IMP
import IMP.em
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    """Class to test EM correlation restraint"""

    def correlation_test(self, data):

        for fn, res, thr in data:
            scene = IMP.em.read_map(self.get_input_file_name(fn), self.mrw)
            scene.get_header_writable().set_resolution(res)
            r = IMP.em.FitRestraint(self.particles, scene)
            score = r.evaluate(False)
            print("EM score (1-CC) = " + str(score), " filename:", fn, " res:", res)
            self.assertLess(
                score,
                thr,
                "the correlation score is not correct")

    def test_compare_fit_score_to_eman2_generated_maps(self):
        # eman2 generated maps
        data = [["Ubi-4.5.mrc", 4.5, 0.005],
                ["Ubi-8.5.mrc", 8.5, 0.005],
                ["Ubi-12.5.mrc", 12.5, 0.005],
                ["Ubi-16.5.mrc", 16.5, 0.005]]
        self.correlation_test(data)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.mrw = IMP.em.MRCReaderWriter()
        self.imp_model = IMP.Model()
        name = self.get_input_file_name("1UBI.pdb")
        print(name)
        self.mh = IMP.atom.read_pdb(name,
                                    self.imp_model, IMP.atom.NonWaterNonHydrogenPDBSelector())
        IMP.atom.add_radii(self.mh)
        IMP.atom.create_rigid_body(self.mh)
        self.particles = []
        self.particles += IMP.core.get_leaves(self.mh)

if __name__ == '__main__':
    IMP.test.main()
