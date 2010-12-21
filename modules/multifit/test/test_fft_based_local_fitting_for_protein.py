import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit

class FFTFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_data(self):
        self.mdl=IMP.Model()
        sel=IMP.atom.CAlphaPDBSelector()
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.mh = IMP.atom.read_pdb(self.get_input_file_name("1kp8_A_1oel_A_fit_.pdb"),
                                    self.mdl,sel)
        self.rb=IMP.atom.setup_as_rigid_body(self.mh)
        self.mh_ref = IMP.atom.read_pdb(self.get_input_file_name("1kp8_A_1oel_A_fit_.pdb"),
                                    self.mdl,sel)
        self.rb_ref = IMP.atom.setup_as_rigid_body(self.mh_ref)
        self.mh_xyz=IMP.core.XYZs(IMP.core.get_leaves(self.mh))
        self.mh_ref_xyz=IMP.core.XYZs(IMP.core.get_leaves(self.mh_ref))
        self.refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        self.scene = IMP.em.read_map(self.get_input_file_name("1oel_A_8.mrc"))
        self.scene.get_header().set_resolution(8)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.load_data()

    def test_fft_based_rigid_fitting_rotations(self):
        """test FFT based local fitting for a protein of three points"""
        #Position the protein center at every possible place on the map
        rots=IMP.algebra.get_uniform_cover_rotations_3d(4)
        print "start"
        fs = IMP.multifit.fft_based_rigid_fitting(
                self.rb,self.refiner,self.scene,0.4,rots,10)
        #check that the score makes sense
        sols=fs.get_solutions()
        sols.sort(True)
        print "number of solutions:",sols.get_number_of_solutions()
        for i in range(sols.get_number_of_solutions()):
            IMP.core.transform(self.rb,sols.get_transformation(i));
            rmsd=IMP.atom.get_rmsd(self.mh_ref_xyz,self.mh_xyz)
            print i,"rmsd:",rmsd," score:",sols.get_score(i)
            IMP.core.transform(self.rb,
                               sols.get_transformation(i).get_inverse());

        sols_clustered=IMP.em.FittingSolutions()
        IMP.multifit.fitting_clustering (self.mh,
                                sols,
                                sols_clustered,
                                self.scene.get_spacing(),200,4)
        sols_clustered.sort(True)
        print "From:",sols.get_number_of_solutions()," clustered to:",sols_clustered.get_number_of_solutions()


        IMP.core.transform(self.rb,sols.get_transformation(0));
        rmsd=IMP.atom.get_rmsd(self.mh_ref_xyz,self.mh_xyz)
        self.assertLess(rmsd,2)
        IMP.core.transform(self.rb,
                           sols.get_transformation(0).get_inverse());
if __name__ == '__main__':
    IMP.test.main()
