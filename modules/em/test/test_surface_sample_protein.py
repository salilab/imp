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
        #init IMP model ( the environment)
        self.mdl = IMP.Model()
        self.mh=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                             self.mdl,IMP.atom.CAlphaSelector())
        IMP.atom.add_radii(self.mh)
        self.particles = IMP.core.get_leaves(self.mh)

    def test_sample_map(self):
        """Check that reading a map back in preserves the stdevs"""
        resolution=1.
        voxel_size=1.
        access_p = IMP.em.IMPParticlesAccessPoint(self.particles,
                              IMP.core.XYZR.get_default_radius_key(),
                              IMP.atom.Mass.get_mass_key())
        print "access points:"
        access_p.show()
        print "=====particles"
        for p in self.particles:
            p.get_particle().show()
        model_map = IMP.em.SurfaceShellDensityMap(access_p, resolution, voxel_size)
        for p in self.particles:
            self.assert_(model_map.get_value(IMP.core.XYZ(p.get_particle()).get_coordinates())>3.,"map was not sampled correctly")

        erw = IMP.em.MRCReaderWriter()
        IMP.em.write_map(model_map, "xxx.mrc",erw)

if __name__ == '__main__':
    unittest.main()
