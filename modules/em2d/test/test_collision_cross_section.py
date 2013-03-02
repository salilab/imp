import IMP
import IMP.test
import IMP.em2d as em2d
import IMP.atom as atom
import IMP.algebra as alg
import IMP.core as core
import IMP.domino as domino
import os
from math import *
import random



class Tests(IMP.test.TestCase):

    def test_ccs_value(self):
        """Test the calculation of the collision cross section of a complex"""
        IMP.base.set_log_level(IMP.base.TERSE)
        m = IMP.Model()

        fn = self.get_input_file_name("1z5s.pdb")
        prot =  atom.read_pdb(fn, m ,atom.ATOMPDBSelector())
        atom.add_radii(prot)
        projections = 20
        resolution = 1.0
        pixel_size = 1.5
        img_size = 80
        ccs = em2d.CollisionCrossSection(projections, resolution, pixel_size, img_size)
        ccs.set_model_particles(IMP.atom.get_leaves(prot))
        ccs_calculated = ccs.get_ccs()
        ccs_value = 3838 # A**2
        # good within 2%
        self.assertAlmostEqual(ccs_calculated, ccs_value, delta=ccs_value*0.02)

if __name__ == '__main__':
    IMP.test.main()
