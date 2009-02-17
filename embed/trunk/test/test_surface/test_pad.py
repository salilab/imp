#!/usr/bin/env python

import unittest

from particles_provider import ParticlesProvider
from EM import MRCReaderWriter,vectorf
from EM import DensityMap, SampledDensityMap,CoarseCCatIntervals
import EM


class PadTests(unittest.TestCase):

    def setUp(self):
        # Initial values and names of files
        prot1_coords='data/1z5s_A_2_157.coord'
        prot2_coords='data/1z5s_C_432_587.coord'
        self.prot1_atoms = ParticlesProvider()
        self.prot1_atoms.read(prot1_coords)
        self.prot2_atoms = ParticlesProvider()
        self.prot2_atoms.read(prot2_coords)
        self.d1 = EM.SampledDensityMap(self.prot1_atoms,3.0,1.0)
        self.d2 = EM.SampledDensityMap(self.prot2_atoms,3.0,1.0)
    def test_map_padding(self):
        h1=self.d1.get_header()
        h2=self.d2.get_header()
        min1=self.d1.get_min_value()
        min2=self.d2.get_min_value()
        max1=self.d1.get_max_value()
        max2=self.d2.get_max_value()
        #fix map dimensions

        nx=max(h1.nx,h2.nx)
        ny=max(h1.ny,h2.ny)
        nz=max(h1.nz,h2.nz)

        self.d1.pad(nx,ny,nz)
        self.d2.pad(nx,ny,nz)

        self.assert_(min1 == self.d1.get_min_value(), "wrong map values after padding")
        self.assert_(min2 == self.d2.get_min_value(), "wrong map values after padding")
        self.assert_(max1 == self.d1.get_max_value(), "wrong map values after padding")
        self.assert_(max2 == self.d2.get_max_value(), "wrong map values after padding")


if __name__=='__main__':
    unittest.main()
