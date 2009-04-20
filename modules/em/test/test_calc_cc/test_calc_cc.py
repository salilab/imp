#test that impEM is able to sample IMP particles on a grid.
import unittest
import IMP
import IMP.test
import IMP.em
import os
import sys

class SampleParticlesTests(IMP.test.TestCase):
    """Test particles sampling"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        in_filename = self.get_input_file_name("three_particles_in.em")

        self.scene = IMP.em.DensityMap()
        mrw = IMP.em.MRCReaderWriter()
        erw = IMP.em.EMReaderWriter()
        self.scene.Read(in_filename,erw)

        self.scene.get_header_writable().resolution = 2.0
        self.scene.std_normalize()


        self.particles = IMP.em.ParticlesProvider()
        self.particles.append(9.,5.,5.,1.,1.)
        self.particles.append(12.,9.,4.,1.,1.)
        self.particles.append(4.,5.,5.,1.,1.)

        self.particles_sampling = IMP.em.SampledDensityMap(
                                              self.particles, 2.0, 1.0)


    def test_sample(self):
        """ test that the create map is reasonable """
        return
        dvx = IMP.Floats();
        dvx.insert(dvx.begin(),3,0.0)
        dvy= IMP.Floats()
        dvy.insert(dvy.begin(),3,0.0)
        dvz= IMP.Floats()
        dvz.insert(dvz.begin(),3,0.0)


        cc = IMP.em.CoarseCC()
        self.scene.get_header_writable().compute_xyz_top();
        print "em_origin: " + str(self.scene.get_header().get_xorigin()) + '  '+ str(self.scene.get_header().get_yorigin()) + '  ' + str(self.scene.get_header().get_zorigin()) + '\n'
        print "model_origin: " + str(self.particles_sampling.get_header().get_xorigin()) + '  '+ str(self.particles_sampling.get_header().get_yorigin()) + '  ' + str(self.particles_sampling.get_header().get_zorigin()) + '\n'

        score=cc.evaluate(
                self.scene,
                self.particles_sampling,
                self.particles,
                dvx,dvy,dvz,
                1.0,
                0
                )
        print 'test_sample ' + str(score)
        self.assert_(score < 0.1, "unexpected cross correlation score")



if __name__ == '__main__':
    unittest.main()
