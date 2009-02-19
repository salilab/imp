import unittest
import os
import IMP
import EM
import IMP.em
import IMP.test
import IMP.utils
import IMP.core

class ToParticlesTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        self.scene = EM.DensityMap()
        erw = EM.EMReaderWriter()
        self.scene.Read(self.get_input_file_name("in.em"), erw)
        self.scene.get_header_writable().set_resolution(3.)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()

        self.load_density_map()

    def test_density2particles(self):
        """Test conversion of a density map into a set of particles """
        m = IMP.Model()
        ps = IMP.Particles()
        IMP.em.density2particles(self.scene,self.scene.get_min_value(),ps,m)
        self.assert_(ps.size() > 0)

if __name__ == '__main__':
    unittest.main()
