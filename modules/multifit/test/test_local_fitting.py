import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit

class FFTFittingTest(IMP.test.TestCase):
    """Class to test FFT local fitting"""

    def load_data(self,resolution):
        self.mp= IMP.atom.read_pdb(self.open_input_file("1z5s.pdb"),
                                   self.imp_model)
        self.mp_ref= IMP.atom.read_pdb(self.open_input_file("1z5s.pdb"),
                                   self.imp_model)
        self.scene=IMP.em.particles2density(IMP.core.get_leaves(self.mp),
                                            resolution,
                                            1.5)
        self.prob=IMP.atom.setup_as_rigid_body(IMP.atom.get_by_type(self.mp,IMP.atom.CHAIN_TYPE)[0])
        IMP.atom.write_pdb(IMP.atom.Hierarchy(self.prob),"ref.pdb")
        self.prob_ref=IMP.atom.setup_as_rigid_body(IMP.atom.get_by_type(self.mp_ref,IMP.atom.CHAIN_TYPE)[0])
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
    def test_fitting_scoring_6(self):
        """test that FFT based fitting scores match FitRestraint results at 6 A"""
        print "score resolution 6"
        self.fitting_scoring_main(6)
    def test_fitting_scoring_15(self):
        """test that FFT based fitting scores match FitRestraint results at 15 A"""
        print "score resolution 15"
        self.fitting_scoring_main(15)

    def fitting_scoring_main(self,resolution):
        self.load_data(resolution)
        rand_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-10.,-10.,-10.),
            IMP.algebra.Vector3D(10.,10.,10.))))
        IMP.core.transform(self.prob,rand_t)
        #TODO - return the local options once problems in local resolved
        for [local,gmm_peaking] in [#[True,True],[True,False]
                                    [False,True]
                                    #,[False,False]
                                    ]:

            print "(score) Testing local:",local," gmm:",gmm_peaking
            self.fitting_scoring(local,gmm_peaking,resolution)

    def fitting_scoring(self,local,gmm_peaking,resolution):
        xyz=IMP.core.XYZs(self.refiner.get_refined(self.prob))
        threshold=0.4
        xyz_ref=IMP.core.XYZs(self.refiner.get_refined(self.prob_ref))
        #fit protein
        rots=IMP.algebra.Rotation3Ds()
        rots.append(IMP.algebra.get_identity_rotation_3d())
        fs = IMP.multifit.fft_based_rigid_fitting(
               self.prob,self.refiner,self.scene,
               threshold,rots,1,local,gmm_peaking)
        #check that the score makes sense
        sols=fs.get_solutions()
        hit_map=fs.get_max_cc_map()
        self.scene.std_normalize()
        for i in range(sols.get_number_of_solutions()):
            IMP.core.transform(self.prob,sols.get_transformation(i));
            print sols.get_transformation(i)
            sampled_dmap=IMP.em.SampledDensityMap(self.scene.get_header())
            sampled_dmap.set_particles(self.refiner.get_refined(self.prob))
            sampled_dmap.resample()
            sampled_dmap.std_normalize()
            IMP.em.write_map(sampled_dmap,"map_sample_"+str(resolution)+".mrc",
                             IMP.em.MRCReaderWriter())
            corr=IMP.em.CoarseCC.local_cross_correlation_coefficient(
                self.scene,
                sampled_dmap,
                sampled_dmap.get_minimum_resampled_value())
            IMP.em.write_map(sampled_dmap,"sampled.mrc",IMP.em.MRCReaderWriter())
            IMP.em.write_map(self.scene,"scene.mrc",IMP.em.MRCReaderWriter())
            rmsd=IMP.atom.get_rmsd(xyz,xyz_ref)
            IMP.core.transform(self.prob,sols.get_transformation(i).get_inverse());
            corr_sol = sols.get_score(i)
            print "score:",corr,corr_sol,rmsd
            self.assertLess(rmsd,4.)
            self.assertGreater(corr,0.8)


if __name__ == '__main__':
    IMP.test.main()
