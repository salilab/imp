import unittest
import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class CropTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_maps(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene= IMP.em.read_map(self.get_input_file_name("1z5s_20.imp.mrc"), mrw)
        self.scene.get_header_writable().set_resolution(20.)
        self.scene.update_voxel_size(3.)
        self.scene.calcRMS()


    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        self.load_density_maps()

    def test_crop_map(self):
        """Test the pad_margin function"""
        dmap2=self.scene.get_cropped(0)
        IMP.em.write_map(dmap2,"test2.mrc",IMP.em.MRCReaderWriter())
        dmap3=IMP.em.read_map("test2.mrc",IMP.em.MRCReaderWriter())
        coarse_cc=IMP.em.CoarseCC()
        IMP.set_log_level(IMP.VERBOSE)
        #check that the center stays in the same place
        self.assertAlmostEqual(IMP.algebra.get_distance(self.scene.get_centroid(),dmap2.get_centroid()),0,2)
        #check that the spacing is correct
        self.assertAlmostEqual(dmap3.get_spacing(),3,2)
        #check that the dmin and dmax are the same
        self.assertAlmostEqual(dmap2.get_min_value()-self.scene.get_min_value(),0.,2)
        self.assertAlmostEqual(dmap2.get_max_value()-self.scene.get_max_value(),0.,2)
        os.unlink("test2.mrc")
if __name__ == '__main__':
    unittest.main()
