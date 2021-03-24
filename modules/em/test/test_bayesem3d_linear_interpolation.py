from __future__ import print_function
import IMP
import IMP.test
import IMP.em
import os, sys
import numpy as np
import random, math

def get_particles(m, input_pdb):
    ### Read input fragment
    sel0 = IMP.atom.BackbonePDBSelector()
    sel1 = IMP.atom.CBetaPDBSelector()
    mh = IMP.atom.read_pdb(input_pdb, m, IMP.atom.OrPDBSelector(sel0, sel1))
    IMP.atom.add_radii(mh)
    return mh, IMP.core.get_leaves(mh)

class Tests(IMP.test.TestCase):
    """Derivative score stability Test"""

    def setUp(self):

        IMP.test.TestCase.setUp(self)


        IMP.set_log_level(IMP.SILENT)
        IMP.set_check_level(IMP.NONE)

        self.x = [1, 2, 3]
        self.y = [3, 2, 0]
    def test_linear_interpolation(self):

        vp = np.interp(2.5, self.x, self.y)
        ip = IMP.em.bayesem3d_linear_interpolate(self.x, self.y, 2.5, False)

        self.assertAlmostEqual(vp, ip, delta=1e-10)

    def test_error_on_size_less_than_two(self):
        x = [1]
        y = [3, 2, 0]

        with self.assertRaises(Exception) as context:
            ip = IMP.em.bayesem3d_linear_interpolate(x, y, 2.5, False)

        self.assertEqual("The box size cannot be smaller than 2", str(context.exception))

    def test_boundaries_on_the_right(self):
        x = [1, 2, 3, 4, 5]
        y = [10, 12, 13, 14, 15]

        ip = IMP.em.bayesem3d_linear_interpolate(x, y, 20, False)

        self.assertAlmostEqual(ip, 15, delta=1e-10)

if __name__ == '__main__':
    IMP.test.main()
