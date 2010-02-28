import unittest
import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class ProteinRigidFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name("1f7dA00_8.mrc"), mrw)
        self.scene.get_header_writable().set_resolution(8.)
        self.scene.update_voxel_size(1.5)
    def load_protein(self,pdb_filename):
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.NonWaterPDBSelector())
        IMP.atom.add_radii(self.mp)
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.particles = IMP.core.get_leaves(self.mp)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1f7dA00.pdb")

    def test_em_local_rigid_fitting_around_point(self):
        """Check that local rigid fitting around a point works"""
        if sys.platform == 'sunos5':
            print >> sys.stderr, "Test skipped: too slow to run on Solaris"
            return
        check = IMP.get_check_level()
        css= IMP.core.ChecksScoreState(.05)
        self.imp_model.add_score_state(css)
        try:
            # This test is super-slow, so disable checks to speed it up a little
            #IMP.set_check_level(IMP.NONE)
            #create a rigid body
            rb_d = IMP.atom.rigid_body_setup_hierarchy(self.mp)
            ref_trans = rb_d.get_transformation()
            fr = IMP.em.FittingSolutions()
            IMP.em.local_rigid_fitting(
                rb_d,self.radius_key, self.weight_key,
                self.scene,
                fr,None,
                2,5,10)
            #todo - add test that if you apply the transformation on the
            # original configuration you get the same result
            # (in rmsd and score)
            print fr.get_number_of_solutions()
            print fr.get_score(0)
            #second, test that the optimization gets you close.
            score = self.imp_model.evaluate(False)
            self.assert_(fr.get_number_of_solutions() == 2,
                         "not enough solutions returned")
            self.assert_(fr.get_score(0) < fr.get_score(1),
                         "solutions are not sorted")
            for i in xrange(2):
                print fr.get_score(i)
                self.assert_(fr.get_score(i) < 1.0, "wrong CC values")
        finally:
            IMP.set_check_level(check)
        print "things were checked " +str(css.get_number_of_checked()) + " times"

if __name__ == '__main__':
    unittest.main()
