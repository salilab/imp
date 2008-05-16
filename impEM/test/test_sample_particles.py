import IMP
import IMP.test
import sys
import IMP.utils
import IMPEM
import EM
import unittest
import os

class SampleTests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = []
        self.em_rs = IMP.RestraintSet("em")
        self.imp_model.add_restraint(self.em_rs)
        ## -  create a set of three particles in imp
        npart = 3
        for i, (x,y,z) in enumerate(((9.0, 9.0, 9.0),
                                     (12.0, 3.0, 3.0),
                                     (3.0, 12.0, 12.0))):
            p = self.create_point_particle(self.imp_model, 0, 0, 0)
            p.add_attribute(IMP.FloatKey("radius"), 1.0, False)
            p.add_attribute(IMP.FloatKey("weight"), 1.0)
            p.add_attribute(IMP.IntKey("id"), i)
            p.add_attribute(IMP.IntKey("protein"), 1)
            self.particles.append(p)
        self.particle_indexes = IMP.Ints()
        self.particle_indexes.clear()
        for i in xrange(npart):
            self.particle_indexes.push_back(i)
        print "initialization done ..."

    def test_sample_map(self):
        """Check that reading a map back in preserves the stdevs"""
        resolution=3.
        voxel_size=1.
        access_p = IMPEM.IMPParticlesAccessPoint(self.imp_model,
                                  self.particle_indexes, "radius", "weight")
        model_map = EM.SampledDensityMap(access_p, resolution, voxel_size)
        model_map.calcRMS()
        erw = EM.EMReaderWriter()
        model_map.Write("xxx.em",erw)
        em_map = EM.DensityMap()
        em_map.Read("xxx.em",erw)
        em_map.calcRMS()
        self.assert_(abs(em_map.get_header().rms - \
                         model_map.get_header().rms) < .000001,
                     "standard deviations of maps differ")
        os.unlink("xxx.em")

if __name__ == '__main__':
    unittest.main()
