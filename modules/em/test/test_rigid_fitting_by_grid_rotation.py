import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import random,math

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name("1z5s_10.mrc"), mrw)
        self.resolution=10.
        self.voxel_size=2.
        self.scene.get_header_writable().set_resolution(self.resolution)
        self.scene.update_voxel_size(self.voxel_size)
        self.scene.set_origin(34.0,8.0,-92.0)
    def load_protein(self,pdb_filename):
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.CAlphaPDBSelector())#IMP.atom.NonWaterSelector())
        IMP.atom.add_radii(self.mp)
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.particles = IMP.core.get_leaves(self.mp)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        IMP.base.set_check_level(IMP.base.NONE)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1z5s_A.pdb")

    def test_cc_after_grid_rotation(self):
        """Check that CC score does not change after grid and protein rotation"""
        check = IMP.base.get_check_level()
        mrw=IMP.em.MRCReaderWriter()
        #create a rigid body
        rb_p = IMP.Particle(self.imp_model)
        #sample density map
        sampled_density_map = IMP.em.SampledDensityMap(self.scene.get_header())
        sampled_density_map.set_particles(self.particles)
        sampled_density_map.resample()
        sampled_density_map.calcRMS()
        IMP.em.write_map(sampled_density_map,self.get_tmp_file_name("a1.mrc"),mrw)
        IMP.em.write_map(self.scene,self.get_tmp_file_name("a2.mrc"),mrw)
        #calculate CC
        ccc = IMP.em.CoarseCC()
        threshold=sampled_density_map.get_header().dmin
        score1 = ccc.cross_correlation_coefficient(self.scene,sampled_density_map,threshold,True)
        print "score1:",score1
        #determine a random transformation
        #t=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
        #                               IMP.algebra.random_vector_in_unit_box())

        #set a small random transformation
        translation = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        axis = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
        rand_angle = random.uniform(-15./180*math.pi,15./180*math.pi)
        r= IMP.algebra.get_rotation_about_axis(axis, rand_angle)
        t=IMP.algebra.Transformation3D(r,translation)

        for p in self.particles:
            IMP.core.XYZ(p).set_coordinates(t.get_transformed(IMP.core.XYZ(p).get_coordinates()))
        transformed_density = IMP.em.get_transformed(self.scene,
                                                     t,threshold)
        sampled_density_map1 = IMP.em.SampledDensityMap(transformed_density.get_header())
        sampled_density_map1.set_particles(self.particles)
        sampled_density_map1.resample()
        sampled_density_map1.calcRMS()
        transformed_density.calcRMS()
        threshold=sampled_density_map1.get_header().dmin
        IMP.em.write_map(sampled_density_map1,self.get_tmp_file_name("b1.mrc"),mrw)
        IMP.em.write_map(transformed_density,self.get_tmp_file_name("b2.mrc"),mrw)
        score2 = ccc.cross_correlation_coefficient(transformed_density,
                                                   sampled_density_map1,
                                                   threshold,True)
        print "score2:",score2
        #move the particles back for the next test
        t_inv = t.get_inverse()
        for p in self.particles:
            IMP.core.XYZ(p).set_coordinates(t_inv.get_transformed(IMP.core.XYZ(p).get_coordinates()))
        self.assertAlmostEqual(score1,score2, delta=0.05) #because rotation the grid changes the density a bit

    def test_cc_with_sampled_grid_rotation(self):
        """CC score does not change after sampled grid and protein rotation"""
        check = IMP.base.get_check_level()

        #set a small random transformation
        translation = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        axis = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
        rand_angle = random.uniform(-15./180*math.pi,15./180*math.pi)
        r= IMP.algebra.get_rotation_about_axis(axis, rand_angle)
        t=IMP.algebra.Transformation3D(r,translation)


        #create a sampled density map of the particles
        sampled_density_map = IMP.em.SampledDensityMap(self.scene.get_header())
        sampled_density_map.set_particles(self.particles)
        sampled_density_map.resample()
        sampled_density_map.calcRMS()
        #rotate the map
        threshold=sampled_density_map.get_header().dmin
        transformed_sampled_density = IMP.em.get_transformed(sampled_density_map,t)


        # transform the particles and resample the original map
        for p in self.particles:
            IMP.core.XYZ(p).set_coordinates(t.get_transformed(IMP.core.XYZ(p).get_coordinates()))
        sampled_density_map.resample()
        sampled_density_map.calcRMS()
        #calculate CC after rotating the particles
        ccc = IMP.em.CoarseCC()

        threshold=sampled_density_map.get_header().dmin
        score1 = ccc.cross_correlation_coefficient(self.scene,sampled_density_map,threshold,True)
        print "score1:",score1


        transformed_sampled_density.calcRMS()
        threshold=transformed_sampled_density.get_header().dmin
        score2 = IMP.em.CoarseCC.cross_correlation_coefficient(
            self.scene,
            transformed_sampled_density,
            transformed_sampled_density.get_header().dmin,True)

        print "score2:",score2
        #move the particles back for the next test
        t_inv = t.get_inverse()
        for p in self.particles:
            IMP.core.XYZ(p).set_coordinates(t_inv.get_transformed(IMP.core.XYZ(p).get_coordinates()))
        print "scores:",score1,":",score2
        self.assertAlmostEqual(score1,score2, delta=0.055)
if __name__ == '__main__':
    IMP.test.main()
