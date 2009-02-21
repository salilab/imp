#!/usr/bin/env python

import unittest

from particles_provider import ParticlesProvider
from EM import MRCReaderWriter,vectorf
from EM import DensityMap, SampledDensityMap,CoarseCCatIntervals
import EM


class SurfaceTests(unittest.TestCase):

    def setUp(self):
        # Initial values and names of files
        prot1_coords='data/1z5s_A_2_157.coord'
        prot2_coords='data/1z5s_C_432_587.coord'
        prot2_bad_coords='data/1z5s_C_432_587_bad.coord'

        self.prot1_atoms = ParticlesProvider()
        self.prot1_atoms.read(prot1_coords)
        self.prot1_shell_map = EM.SurfaceShellDensityMap(self.prot1_atoms,2.0,8)
        self.prot1_shell_map.resample(self.prot1_atoms)
        self.prot2_atoms = ParticlesProvider()
        self.prot2_atoms.read(prot2_coords)
        self.prot2_shell_map = EM.SurfaceShellDensityMap(self.prot2_atoms,2.0,8)
        self.prot2_shell_map.resample(self.prot2_atoms)
        self.prot2_bad_atoms = ParticlesProvider()
        self.prot2_bad_atoms.read(prot2_bad_coords)
        self.prot2_bad_shell_map = EM.SurfaceShellDensityMap(self.prot2_bad_atoms,2.0,8)
        self.prot2_bad_shell_map.resample(self.prot2_bad_atoms)

    def test_good_geo_comp(self):
        erw = EM.EMReaderWriter()
#        self.prot1_shell_map.Write("prot1_shell.em",erw)
#        self.prot2_shell_map.Write("prot2_shell.em",erw)
         #fix map dimensions
        h1=self.prot1_shell_map.get_header()
        h2=self.prot2_shell_map.get_header()
        nx=max(h1.nx,h2.nx)
        ny=max(h1.ny,h2.ny)
        nz=max(h1.nz,h2.nz)

        self.prot1_shell_map.pad(nx,ny,nz)
        self.prot2_shell_map.pad(nx,ny,nz)

        conv = EM.CoarseCC()
        score = conv.cross_correlation_coefficient(self.prot1_shell_map,
                                                   self.prot2_shell_map,
                                                   0.0,False,False)
        print "SCORE : " + str(score)
    def test_bad_geo_comp(self):
        erw = EM.EMReaderWriter()
        #        self.prot1_shell_map.Write("prot1_shell.em",erw)
        #        self.prot2_shell_map.Write("prot2_shell.em",erw)
        #fix map dimensions
        h1=self.prot1_shell_map.get_header()
        h2=self.prot2_bad_shell_map.get_header()
        nx=max(h1.nx,h2.nx)
        ny=max(h1.ny,h2.ny)
        nz=max(h1.nz,h2.nz)

        self.prot1_shell_map.pad(nx,ny,nz)
        self.prot2_bad_shell_map.pad(nx,ny,nz)

        conv = EM.CoarseCC()
        score = conv.cross_correlation_coefficient(self.prot1_shell_map,
                                                   self.prot2_bad_shell_map,
                                                   0.0,False,False)
        print "SCORE BAD : " + str(score)
if __name__=='__main__':
    unittest.main()
