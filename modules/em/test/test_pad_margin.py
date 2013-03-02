import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_maps(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene= IMP.em.read_map(self.get_input_file_name("1z5s_20.imp.mrc"), mrw)
        self.scene.get_header_writable().set_resolution(20.)
        self.scene.update_voxel_size(2.5)
        self.scene.calcRMS()


    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        self.load_density_maps()

    def test_pad_margin(self):
        """Test the pad_margin function"""
        scene_padded=self.scene.pad_margin(3,3,3)
        self.scene.calcRMS()
        scene_padded.calcRMS()
        #test that the centers are the same
        padded_h = scene_padded.get_header()
        h = self.scene.get_header()
        apix=h.get_spacing()

        scene_center = self.scene.get_centroid(0.1)
        padded_scene_center=scene_padded.get_centroid(0.1)

        print scene_center,padded_scene_center
        print IMP.algebra.get_distance(padded_scene_center,scene_center)

        self.assertAlmostEqual(IMP.algebra.get_distance(padded_scene_center,scene_center),0,1)
if __name__ == '__main__':
    IMP.test.main()
