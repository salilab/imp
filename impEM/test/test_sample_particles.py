import IMP
import sys
import IMP.utils
import IMPEM
import EM
import unittest
import os

def init_particle(particles,p_ind_,x_,y_,z_,r_=0.0,w_=1.0,protein_=1):
    """init_particle(particles_,p_ind_,x_,y_,z_,r_=0.0,w=1.0,protein_=1)
    particles     IMP particles
    p_ind         particle indices
    x_            x coord
    y_            y coord
    z_            z coord
    r_            radius
    w_            weight
    protein_      protein identifier (int)"""
    p1 = particles[p_ind_]
    p1.add_attribute(IMP.FloatKey("radius"), r_, False)
    p1.add_attribute(IMP.FloatKey("weight"), w_)
    p1.add_attribute(IMP.IntKey("id"), p_ind_)
    particles[p_ind_].set_x(x_)
    particles[p_ind_].set_y(y_)
    particles[p_ind_].set_z(z_)
    p1.add_attribute(IMP.IntKey("protein"), protein_)

class SampleTests(unittest.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = []
        self.em_rs = IMP.RestraintSet("em")
        self.imp_model.add_restraint(self.em_rs)
        ## -  create a set of three particles in imp
        npart = 3
        for i in range(npart):
            self.particles.append(IMP.utils.XYZParticle(self.imp_model, 0,0,0))
        # - add the particles attributes ( other than X,Y,Z)
        rad = 1.0
        wei = 1.0
        init_particle(self.particles,0,9.0,9.0,9.0,rad,wei,1)
        init_particle(self.particles,1,12.0,3.0,3.0,rad,wei,1)
        init_particle(self.particles,2,3.0,12.0,12.0,rad,wei,1)
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
