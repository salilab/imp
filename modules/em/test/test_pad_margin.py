import unittest
import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class PadMarginTest(IMP.test.TestCase):
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

    def test_pad_margin(self):
        """Test the pad_margin function"""
        scene_padded=self.scene.pad_margin(3,3,3)
        print "====||||======"
        scene_padded.show()
        self.scene.calcRMS()
        print "==========",self.scene.get_header().rms
        scene_padded.calcRMS()
        print "==========",scene_padded.get_header().rms
        #test that the centers are the same
        padded_h = scene_padded.get_header()
        h = self.scene.get_header()
        apix=h.get_spacing()

        scene_center = IMP.algebra.Vector3D(h.get_xorigin(),h.get_yorigin(),h.get_zorigin())+IMP.algebra.Vector3D(h.get_nx()/2.*apix,h.get_ny()/2.*apix,h.get_nz()/2.*apix)
        padded_scene_center = IMP.algebra.Vector3D(padded_h.get_xorigin(),padded_h.get_yorigin(),padded_h.get_zorigin())+IMP.algebra.Vector3D(padded_h.get_nx()/2.*apix,padded_h.get_ny()/2.*apix,padded_h.get_nz()/2.*apix)
        self.assertAlmostEqual(IMP.algebra.get_distance(padded_scene_center,scene_center),0,2)
if __name__ == '__main__':
    unittest.main()
