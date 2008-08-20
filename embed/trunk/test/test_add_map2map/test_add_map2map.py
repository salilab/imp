#test that impEM is able to sample IMP particles on a grid.

import unittest
import EM
import os
import sys
class AddParticlesTest(unittest.TestCase):
    """test adding two maps"""

    def setUp(self):
        print 'start setup'
        """ create particles """
        self.particles_p = EM.ParticlesProvider()
        self.particles_p.append(9.,5.,5.,1.,1.)
        self.particles_p.append(12.,9.,4.,1.,1.)
        self.particles_p.append(4.,5.,5.,1.,1.)
        resolution = 2.0
        voxel_size = 1.0
        self.scene1 = EM.SampledDensityMap(self.particles_p,
                                     resolution,voxel_size)

        self.scene2 = EM.SampledDensityMap(self.particles_p,
                                           resolution,voxel_size)

        self.scene3 = EM.SampledDensityMap(self.particles_p,
                                           resolution,voxel_size*2)



    def test_addition(self):
        """ test that adding a map to each other works """
        self.scene1.add(self.scene2)
        for i in [4931,5391,5396,5477,5968]:
            x = self.scene1.voxel2loc(i,0)
            y = self.scene1.voxel2loc(i,1)
            z = self.scene1.voxel2loc(i,2)
            self.assert_(self.scene1.get_value(x,y,z) == 2 * self.scene2.get_value(x,y,z), "unexpected value after addition")

if __name__ == '__main__':
    unittest.main()
