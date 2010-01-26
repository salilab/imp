#test that impEM is able to sample IMP particles on a grid.

import unittest
import IMP.em
import IMP.test
import os
import sys

class AddParticlesTest(IMP.test.TestCase):
    """test adding two maps"""

    def setUp(self):
        """ create particles """
        IMP.test.TestCase.setUp(self)
        self.mdl=IMP.Model()
        self.particles_p = IMP.Particles()
        self.rad_key=IMP.core.XYZR.get_default_radius_key()
        self.mass_key=IMP.FloatKey("mass")
        for val in [[9.,5.,5.,1.,1.],[12.,9.,4.,1.,1.],[4.,5.,5.,1.,1.]]:
            p=IMP.Particle(self.mdl)
            IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(val[0],val[1],val[2]),val[3]))
            p.add_attribute(self.mass_key,5.)
            self.particles_p.append(p)
        resolution = 2.0
        voxel_size = 1.0
        self.scene1 = IMP.em.SampledDensityMap(self.particles_p,
                                               resolution,voxel_size)

        self.scene2 = IMP.em.SampledDensityMap(self.particles_p,
                                               resolution,voxel_size)

        self.scene3 = IMP.em.SampledDensityMap(self.particles_p,
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
