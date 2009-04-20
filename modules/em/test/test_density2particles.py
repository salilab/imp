import unittest
import os
import IMP
import IMP.em
import IMP.test
import IMP.utils
import IMP.core

class ToParticlesTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_maps(self):
        self.scene1 = IMP.em.DensityMap()
        self.scene2 = IMP.em.DensityMap()
        mrw = IMP.em.MRCReaderWriter()
        erw = IMP.em.EMReaderWriter()
        print self.get_input_file_name("in.em")
        self.scene1.Read(self.get_input_file_name("in.em"), erw)
        self.scene2.Read(self.get_input_file_name("1z5s.mrc"), mrw)
        self.scene1.get_header_writable().set_resolution(3.)
        self.scene2.get_header_writable().set_resolution(10.)
        self.scene2.update_voxel_size(3.0)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        self.load_density_maps()

    def _test_density2particles_map1(self):
        # Disabled
        m = IMP.Model()
        ps = IMP.Particles()
        IMP.em.density2particles(self.scene1,self.scene1.get_min_value()+0.1,ps,m)
        self.assert_(ps.size() > 0)

    def test_density2particles_map2(self):
        """Test conversion of a density map into a set of particles
           This is done after updating the voxel size of the map"""
        m = IMP.Model()
        ps = IMP.Particles()
        self.scene1.get_header().show()
        IMP.em.density2particles(self.scene2,9.0,ps,m)
        self.assert_(ps.size() > 0)

if __name__ == '__main__':
    unittest.main()
