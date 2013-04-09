import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import random

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self,em_filename):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name(em_filename), mrw)
        self.scene.get_header_writable().set_resolution(10.)
        self.scene.update_voxel_size(2.0)

    def load_proteins(self,pdb_filenames):
        self.mhs=IMP.atom.Hierarchies()
        self.ps = []
        self.rbs=IMP.core.RigidBodies()
        self.leaves_ref = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        for pf in pdb_filenames:
            self.mhs.append(IMP.atom.read_pdb(self.open_input_file(pf),
                                       self.imp_model, IMP.atom.CAlphaPDBSelector()))
        for mh in self.mhs:
            IMP.atom.add_radii(mh)
            IMP.atom.create_rigid_body(mh)
            rb=IMP.core.RigidMember(IMP.core.get_leaves(mh)[0]).get_rigid_body()
            self.ps = self.ps + self.leaves_ref.get_refined(mh)
            self.rbs.append(rb)
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        IMP.base.set_check_level(IMP.base.NONE)
        self.imp_model = IMP.Model()
        self.load_density_map("1z5s_10.mrc")
        self.load_proteins(["1z5s_A_fitted.pdb","1z5s_B_fitted.pdb",
                            "1z5s_C_fitted.pdb","1z5s_D_fitted.pdb",])
        self.full_sampled_map=IMP.em.SampledDensityMap(self.scene.get_header())
        self.all_ps=[]
        for mh in self.mhs:
            self.all_ps+=IMP.core.get_leaves(mh)
        self.full_sampled_map.set_particles(self.all_ps)
        self.full_sampled_map.resample()
        self.scene.calcRMS()
        self.full_sampled_map.calcRMS()
        self.upper=(self.scene.get_number_of_voxels()*self.scene.get_header().dmean*self.full_sampled_map.get_header().dmean)/len(self.mhs)
        self.lower=self.scene.get_number_of_voxels()*self.scene.calcRMS()*self.full_sampled_map.calcRMS()
        self.norm_factors=[self.upper,self.lower]

    def test_normalization(self):
        """Test that the normalization factor is a constant"""
        #resample the map for different configuration and see that the normalization factor stays the same
        all_t=[]
        for i in range(10):
            #ramdominze the proteins
            t=[]
            sphere=IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),1)
            for mh in self.mhs:
                rand_translation = 5.*IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
                rand_rot= IMP.algebra.get_rotation_about_axis(
                    IMP.algebra.get_random_vector_in(sphere),random.uniform(-0.3,0.3))
                t.append(IMP.algebra.Transformation3D(rand_rot,rand_translation))
            all_t.append(t)
        rms=[]
        for i in range(10):
            t=all_t[i]
            #transform the proteins
            for j,rb in enumerate(self.rbs):
                IMP.core.transform(rb,t[j])
            self.full_sampled_map.resample()
            rms.append(self.full_sampled_map.calcRMS())
            for j,rb in enumerate(self.rbs):
                IMP.core.transform(rb,t[j].get_inverse())
        for i in range(9):
            self.assertAlmostEqual(rms[i],rms[i+1],2)

    def test_correction_vs_decompose_correlation(self):
        """Test that correlation and decomposed correlation return the same score"""
        cc=IMP.em.CoarseCC()
        #generate all density maps:
        sampled_maps=[]
        for mh in self.mhs:
            sm=IMP.em.SampledDensityMap(self.scene.get_header())
            sm.set_particles(IMP.core.get_leaves(mh))
            sm.resample()
            sampled_maps.append(sm)
        #full sampled map
        decomposed_score=0.
        for i in range(len(self.mhs)):
            decomposed_score += cc.cross_correlation_coefficient(self.scene,
                                                                 sampled_maps[i],0.,False,self.norm_factors)
        print "decomposed_score_norm:",decomposed_score
        print "full score:",cc.cross_correlation_coefficient(self.scene,
                                               self.full_sampled_map,0.,False)
        self.assertAlmostEqual(decomposed_score,
                               cc.cross_correlation_coefficient(self.scene,
                                                                self.full_sampled_map,0.,False),
                               2)


    def test_fit_restraint_decomposition(self):
        """Test that the full and decomposed fit restraint return the same score"""
        cc=IMP.em.CoarseCC()
        #generate all density maps:
        sampled_maps=[]
        for mh in self.mhs:
            sm=IMP.em.SampledDensityMap(self.scene.get_header())
            sm.set_particles(IMP.core.get_leaves(mh))
            sm.resample()
            sm.calcRMS()
            sampled_maps.append(sm)
        #full sampled map
        decomposed_score=0.
        for i in range(len(self.mhs)):
            print "iindex:",i,"mol size",len(IMP.core.get_leaves(self.mhs[i]))
            r=IMP.em.FitRestraint(IMP.core.get_leaves(self.mhs[i]),self.scene,
                                  self.norm_factors,
                                  IMP.atom.Mass.get_mass_key(),
                                  1.,False)
            print "finish set fit restraint"
            self.imp_model.add_restraint(r)
            print "add rstraint"
            decomposed_score += r.evaluate(None)
            print "after evaluate"
        full_r=IMP.em.FitRestraint(self.all_ps,self.scene,[0,0],
                                   IMP.atom.Mass.get_mass_key(),
                                  1.,False)
        self.imp_model.add_restraint(full_r)
        full_score=full_r.evaluate(None)
        print "decomposed score:",decomposed_score-(len(self.mhs)-1)
        print "decomposed score normalized:",full_score
        self.assertAlmostEqual(decomposed_score-(len(self.mhs)-1),full_score,1)

if __name__ == '__main__':
    IMP.test.main()
