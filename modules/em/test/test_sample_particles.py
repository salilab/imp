import IMP
import IMP.test
import sys
import IMP.em
import unittest
import os

class SampleTests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = IMP.Particles()
        ## -  create a set of three particles in imp
        npart = 3
        self.rad_key=IMP.FloatKey("radius")
        self.weight_key=IMP.FloatKey("weight")
        for i, (x,y,z) in enumerate(((9.0, 9.0, 9.0),
                                     (12.0, 3.0, 3.0),
                                     (3.0, 12.0, 12.0))):
            p = self.create_point_particle(self.imp_model, x,y,z)
            p.add_attribute(self.rad_key, 1.0, False)
            p.add_attribute(self.weight_key, 10.0)
            p.add_attribute(IMP.IntKey("id"), i)
            p.add_attribute(IMP.IntKey("protein"), 1)
            self.particles.append(p)
        self.particle_indexes = IMP.Ints()
        self.particle_indexes.clear()
        for i in range(npart):
            self.particle_indexes.push_back(i)
        print "initialization done ..."

    def test_sample_map(self):
        """Check that sampling particles works"""
        erw=IMP.em.EMReaderWriter()
        resolution=1.
        voxel_size=1.
        model_map = IMP.em.SampledDensityMap(self.particles, resolution, voxel_size,self.rad_key,self.weight_key)
        IMP.em.write_map(model_map,"a.em",erw)
        for p in self.particles:
            v=IMP.core.XYZ(p).get_coordinates()
            print model_map.get_value(v)
            self.assert_(model_map.get_value(v)>0.6,
                         "map was not sampled correctly")
        model_map.calcRMS()
        IMP.em.write_map(model_map, "xxx.em",erw)
        em_map = IMP.em.DensityMap()
        em_map= IMP.em.read_map("xxx.em",erw)
        em_map.calcRMS()
        self.assert_(abs(em_map.get_header().rms - \
                         model_map.get_header().rms) < .000001,
                     "standard deviations of maps differ")
        os.unlink("xxx.em")

if __name__ == '__main__':
    unittest.main()
