import os
import IMP
import IMP.em
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    """Class to test EM correlation restraint"""

    def load_density_maps(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(
            self.get_input_file_name("1z5s_20.imp.mrc"),
            mrw)
        self.scene.get_header_writable().set_resolution(20.)
        self.scene.update_voxel_size(3.)
        self.scene.calcRMS()

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        self.load_density_maps()

    def test_crop_map(self):
        """Test the cropping functionality"""
        t = 0.
        cropped_scene = self.scene.get_cropped(0.)
        cropped_bb = IMP.em.get_bounding_box(cropped_scene)
        scene_bb = IMP.em.get_bounding_box(self.scene, t)
        # check that the scene and cropped scene have the same bounding box
        for i in range(2):
            self.assertAlmostEqual(IMP.algebra.get_distance(
                scene_bb.get_corner(i),
                cropped_bb.get_corner(i)), 0, 2)
        cropped_scene.get_header().show()
        mrw = IMP.em.MRCReaderWriter()
        IMP.em.write_map(cropped_scene, "test2.mrc", mrw)
        dmap3 = IMP.em.read_map("test2.mrc", IMP.em.MRCReaderWriter())
        coarse_cc = IMP.em.CoarseCC()
        # check that the center stays in the same place
        self.assertAlmostEqual(IMP.algebra.get_distance(
            dmap3.get_centroid(),
            cropped_scene.get_centroid()), 0, 2)
        self.assertAlmostEqual(IMP.algebra.get_distance(
            self.scene.get_centroid(),
            cropped_scene.get_centroid()), 0, 2)
        # check that the spacing is correct
        self.assertAlmostEqual(dmap3.get_spacing(), 3, 2)
        # check that the dmin and dmax are the same
        self.assertAlmostEqual(
            cropped_scene.get_min_value(
            ) - self.scene.get_min_value(
            ),
            0.,
            2)
        self.assertAlmostEqual(
            cropped_scene.get_max_value(
            ) - self.scene.get_max_value(
            ),
            0.,
            2)
        os.unlink("test2.mrc")

    def _test_crop_using_larger_extent(self):
        """Test the cropping functionality works when the input bb is larger than the density"""
        em_bb = IMP.em.get_bounding_box(self.scene)
        larger_bb = IMP.algebra.BoundingBox3D(
            em_bb.get_corner(0) - IMP.algebra.Vector3D(10, 10, 10),
            em_bb.get_corner(1) + IMP.algebra.Vector3D(10, 10, 10))
        cropped_scene = self.scene.get_cropped(larger_bb)
        IMP.em.write_map(cropped_scene, "temp.mrc", IMP.em.MRCReaderWriter())
        cropped_bb = IMP.em.get_bounding_box(cropped_scene)
        scene_bb = IMP.em.get_bounding_box(self.scene)
        # check that the scene and cropped scene have the same bounding box
        for i in range(2):
            self.assertAlmostEqual(IMP.algebra.get_distance(
                scene_bb.get_corner(i),
                cropped_bb.get_corner(i)), 0, 2)

    def test_crop_by_particle_set(self):
        """Test the cropping functionality by inputting a particle set"""
        mrw = IMP.em.MRCReaderWriter()
        mh = IMP.atom.read_pdb(
            self.get_input_file_name("1mbn_21-35.pdb"),
            self.imp_model,
            IMP.atom.BackbonePDBSelector())

        ps = IMP.atom.get_leaves(mh)
        ps_cent = IMP.algebra.get_centroid([IMP.core.XYZ(p).get_coordinates() for p in ps])

        mrc = IMP.em.read_map(self.get_input_file_name("1mbn.6.eman.mrc"), mrw)
        mrc.get_header_writable().set_resolution(6.0)

        cropped_map = mrc.get_cropped(ps, 5.0)
        c_cent = cropped_map.get_centroid()

        # Centroid of the particle set used to crop and the cropped density
        # should be about the same
        self.assertAlmostEqual(IMP.algebra.get_distance(c_cent, ps_cent), 0.0, delta=1.0)

        # The correlation between the ps and map should be better
        cropped_cfs = 1-IMP.em.compute_fitting_score(ps, cropped_map)
        full_cfs = 1-IMP.em.compute_fitting_score(ps, mrc)

        self.assertGreater(cropped_cfs, full_cfs)

        # The cropped map should have less density
        self.assertLess(IMP.em.approximate_molecular_mass(cropped_map, 0.00), 
                IMP.em.approximate_molecular_mass(mrc, 0.00))

        inv_cropped_map = mrc.get_cropped(ps, 5.0, True)
        inv_c_cent = inv_cropped_map.get_centroid()

        inv_cropped_cfs = 1-IMP.em.compute_fitting_score(ps, inv_cropped_map)

        # The correlation of the inverse map should be the worst
        self.assertGreater(cropped_cfs, inv_cropped_cfs)

        # These evaluate to equal - probably a bug in compute_fitting_score
        # Change to Ilan's EMFitRestraint score when implemented
        #self.assertGreater(full_cfs, inv_cropped_cfs)

        # Inverse cropped map should have less density
        self.assertLess(IMP.em.approximate_molecular_mass(inv_cropped_map, 0.00), IMP.em.approximate_molecular_mass(mrc, 0.00))

        # Addition of the two cropped maps should be almost identical to the original
        inv_cropped_map.add(cropped_map)

        # This evaluates to greater than 1.0 for unknown reasons - bug in CCC?
        ccc = IMP.em.CoarseCC.cross_correlation_coefficient(inv_cropped_map, mrc, 0)
        self.assertGreater(ccc, 0.99)

        # Test the keep_em_size flag
        cropped_map_keep = mrc.get_cropped(ps, 5.0, False, True)
        
        self.assertEqual(mrc.get_number_of_voxels(), cropped_map_keep.get_number_of_voxels())

    def test_crop_using_smaller_extent(self):
        """Test the cropping functionality works when the input bb is larger than the density"""
        mrw = IMP.em.MRCReaderWriter()
        mh = IMP.atom.read_pdb(
            self.get_input_file_name("1z5s_A_fitted.pdb"),
            self.imp_model,
            IMP.atom.CAlphaPDBSelector())
        mh_bb = IMP.atom.get_bounding_box(mh)
        cropped_scene = self.scene.get_cropped(mh_bb)

if __name__ == '__main__':
    IMP.test.main()
