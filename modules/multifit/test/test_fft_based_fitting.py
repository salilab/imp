import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.restrainer


class FFTFittingTest(IMP.test.TestCase):
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
        self.mp_ref= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.NonWaterPDBSelector())
        IMP.atom.add_radii(self.mp_ref)

        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.ps = IMP.Particles(IMP.core.get_leaves(self.mp))
        self.rb=IMP.atom.setup_as_rigid_body(self.mp)
        self.refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1f7dA00.pdb")


    def test_fft_based_rigid_fitting_translation_for_protein(self):
        """test FFT based fitting for a protein"""
        #randomize protein placement
        rand_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-10.,-10.,-10.),
            IMP.algebra.Vector3D(10.,10.,10.))))

        xyz=IMP.core.XYZsTemp(self.ps)
        IMP.core.transform(self.rb,rand_t)
        #IMP.atom.write_pdb(self.mp,"translated.pdb")
        xyz_ref=IMP.core.XYZsTemp(IMP.core.get_leaves(self.mp_ref))
        #fit protein
        fs = IMP.multifit.fft_based_rigid_fitting(
               self.rb,self.refiner,self.scene,0.15,1,10)
        #check that the score makes sense
        sols=fs.get_solutions()
        #self.assertAlmostEqual(score,1.,
        #check that the rmsd is low
        best_rmsd=9999
        best_score=0
        best_trans=""
        for i in range(sols.get_number_of_solutions()):
            IMP.core.transform(self.rb,sols.get_transformation(i));
            rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
            IMP.core.transform(self.rb,sols.get_transformation(i).get_inverse());
            if best_rmsd>rmsd:
                best_rmsd=rmsd
                best_score=sols.get_score(i)
                best_trans=sols.get_transformation(i)
        cc_map=fs.get_max_cc_map()
        IMP.em.write_map(cc_map,"max.mrc",IMP.em.MRCReaderWriter())

        print "PDB best RMSD:",best_rmsd
        print "SCORE:",best_score
        print "BEST TRANS:",sols.get_transformation(i)
        self.assertAlmostEqual(best_rmsd,0.,delta=1.)
        self.assertAlmostEqual(best_score,1.,delta=1.)


    def create_points(self,mdl):
        rd= IMP.core.XYZ.setup_particle(IMP.Particle(mdl),
                                        IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        hd= IMP.core.Hierarchy.setup_particle(rd.get_particle())
        ps = IMP.Particles()
        origin =  3.0
        ps.append(self.create_point_particle(mdl,
                                             9.+origin, 9.+origin,
                                             9.+origin))
        ps.append(self.create_point_particle(mdl,
                                             12.+origin, 3.+origin,
                                             3.+origin))
        ps.append(self.create_point_particle(mdl,
                                             3.+origin, 12.+origin,
                                             12.+origin))
        for i in range(3):
            p1 = ps[i]
            p1.add_attribute(self.radius_key, 1.0)
            p1.add_attribute(self.weight_key, 1.0)
            chd=IMP.core.Hierarchy.setup_particle(p1)
            hd.add_child(chd)
        rb=IMP.core.RigidBody.setup_particle(rd.get_particle(),hd.get_children())
        return [rb,IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())]
        #make all a rigid body
    def test_resampling(self):
        """Test resampling consistency"""
        mdl=IMP.Model()
        [rb,refiner]=self.create_points(mdl)
        ps_xyz =  IMP.core.XYZsTemp(refiner.get_refined(rb))
        dmap=IMP.em.particles2density(ps_xyz,3,1)
        dmap.calcRMS()
        dmap2=IMP.em.SampledDensityMap(dmap.get_header())
        dmap2.set_particles(ps_xyz)
        dmap2.resample()
        self.assertAlmostEqual(IMP.em.CoarseCC.cross_correlation_coefficient(dmap,dmap2,0),1,1)

    def test_fft_round_trip(self):
        """Test FFT round trip"""
        mdl=IMP.Model()
        [rb,refiner]=self.create_points(mdl)
        ps_xyz =  IMP.core.XYZsTemp(refiner.get_refined(rb))
        dmap=IMP.em.particles2density(ps_xyz,3,1)
        dmap.calcRMS()
        fft_fit=IMP.multifit.FFTFitting(dmap,rb,refiner);
        fft_fit.prepare(0)
        fft_fit.recalculate_molecule()
        map2=fft_fit.test_fftw_round_trip()
        map2.calcRMS()
        #test that the correlation is 1
        cc=IMP.em.CoarseCC.cross_correlation_coefficient(dmap,map2,0)
        self.assertAlmostEqual(IMP.em.CoarseCC.cross_correlation_coefficient(dmap,map2,0),1,1)

    def test_fft_based_rigid_fitting_translation(self):
        """test FFT based fitting on 3 particles"""
        mdl=IMP.Model()
        [rb,refiner]=self.create_points(mdl)
        [rb_ref,refiner_ref]=self.create_points(mdl)
        #create map
        dmap=IMP.em.particles2density(refiner.get_refined(rb),3,1)
        #generate a 3 particle object
        #randomize protein placement
        rand_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-10.,-10.,-10.),
            IMP.algebra.Vector3D(10.,10.,10.))))

        xyz=IMP.core.XYZsTemp(refiner.get_refined(rb))
        IMP.core.transform(rb,rand_t)
        xyz_ref=IMP.core.XYZsTemp(refiner_ref.get_refined(rb_ref))
        #fit protein
        fs = IMP.multifit.fft_based_rigid_fitting(
               rb,refiner,dmap,0,1)
        #check that the score makes sense
        sols=fs.get_solutions()
        score=sols.get_score(0)
        #self.assertAlmostEqual(score,1.,
            #check that the rmsd is low
        best_rmsd=9999
        best_score=0
        for i in range(sols.get_number_of_solutions()):
            IMP.core.transform(rb,sols.get_transformation(i));
            rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
            IMP.core.transform(rb,sols.get_transformation(i).get_inverse())
            if rmsd<best_rmsd:
                best_rmsd=rmsd
                best_score=sols.get_score(i)
        print "RMSD:",best_rmsd
        print "SCORE:",best_score
        cc_map=fs.get_max_cc_map()
        IMP.em.write_map(cc_map,"max2.mrc",IMP.em.MRCReaderWriter())
        self.assertAlmostEqual(best_rmsd,0.,delta=1.)
        self.assertAlmostEqual(best_score,1.,delta=1.)


    def test_fft_based_rigid_fitting_rotation_translation(self):
        """test FFT based fitting on 3 particles with 10 rotations"""
        mdl=IMP.Model()
        [rb,refiner]=self.create_points(mdl)
        [rb_ref,refiner_ref]=self.create_points(mdl)
        #create map
        dmap=IMP.em.particles2density(refiner.get_refined(rb),3,1)
        #generate a 3 particle object
        #randomize protein placement
        rand_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
            IMP.algebra.BoundingBox3D(
            IMP.algebra.Vector3D(-10.,-10.,-10.),
            IMP.algebra.Vector3D(10.,10.,10.))))
        xyz=IMP.core.XYZsTemp(refiner.get_refined(rb))
        xyz_ref=IMP.core.XYZsTemp(refiner_ref.get_refined(rb_ref))
        IMP.core.transform(rb,rand_t)
        start_rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
        print "ROT START RMSD:", IMP.atom.get_rmsd(xyz_ref,xyz)

        #fit protein
        fs = IMP.multifit.fft_based_rigid_fitting(
               rb,refiner,dmap,0,10)
        #check that the score makes sense
        sols=fs.get_solutions()
        score=sols.get_score(0)
        #self.assertAlmostEqual(score,1.,
            #check that the rmsd is low
        best_rmsd=9999
        best_score=0
        for i in range(sols.get_number_of_solutions()):
            IMP.core.transform(rb,sols.get_transformation(i));
            rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
            IMP.core.transform(rb,sols.get_transformation(i).get_inverse())
            if rmsd<best_rmsd:
                best_rmsd=rmsd
                best_score=sols.get_score(i)
        print "ROT RMSD:",best_rmsd
        print "ROT SCORE:",best_score
        cc_map=fs.get_max_cc_map()
        self.assertLess(best_rmsd,start_rmsd)



if __name__ == '__main__':
    IMP.test.main()
