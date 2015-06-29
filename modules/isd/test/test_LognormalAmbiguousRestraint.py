#!/usr/bin/env python

import IMP
import IMP.algebra
import IMP.core
from IMP.isd import Nuisance, Scale, LognormalAmbiguousRestraint
import math
import IMP.test

class Tests(IMP.test.TestCase):

    def test_simple(self):
        """Simple test of LognormalAmbiguousRestraint"""
        m = IMP.Model()
        p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                         IMP.algebra.Vector3D(0,0,0))
        p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                         IMP.algebra.Vector3D(3,3,3))
        ki = Scale.setup_particle(IMP.Particle(m), 0.0)
        ki.set_lower(0.)
        ki.set_upper(0.)
        sigma = Scale.setup_particle(IMP.Particle(m), 2.0)
        omega = Scale.setup_particle(IMP.Particle(m), 1.0)
        omega.set_lower(1.)
        omega.set_upper(10000.)
        lnar = IMP.isd.LognormalAmbiguousRestraint(p1, p2, ki, sigma)
        lnar.add_contribution(50., omega)
        self.assertAlmostEqual(lnar.evaluate(True), 6.165, delta=0.001)
        self.assertAlmostEqual(-math.log(lnar.get_probability()),
                               lnar.evaluate(False), delta=1e-4)
        self.assertEqual(lnar.get_number_of_contributions(), 1)

if __name__ == '__main__':
    IMP.test.main()
