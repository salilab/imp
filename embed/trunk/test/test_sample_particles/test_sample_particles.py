#test that impEM is able to sample IMP particles on a grid.



import unittest
import EM
import os

class SampleParticlesTest(unittest.TestCase):
    """test particles sampling  """

    def setUp(self):
        print 'start setup'
        """ create particles """
        self.particles_p = EM.ParticlesProvider()
        self.particles_p.append(9.,5.,5.,1.,1.)
        self.particles_p.append(12.,9.,4.,1.,1.)
        self.particles_p.append(4.,5.,5.,1.,1.)
        print 'after setup SampleParticlesTest'

    def test_sample(self):
        """ test that the sampling works """
        resolution = 2.0
        voxel_size = 1.0
        out_filename = "aa.em"

        scene = EM.SampledDensityMap(self.particles_p,
                                     resolution,voxel_size)

        scene.std_normalize()

        erw = EM.EMReaderWriter()
        scene.Write(out_filename,erw)
        # make sure that the output file is created
        os.unlink(out_filename)


        # the mean and std are not correct
        self.assert_(scene.get_header().dmean == 0.0, "unexpected mean for the map")
        self.assert_(scene.get_header().rms == 1.0, "unexpected rms for the map")

if __name__ == '__main__':
    unittest.main()
