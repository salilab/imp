#!/usr/bin/env python

import math
#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Nuisance

#unit testing framework
import IMP.test

class TestNuisanceParam(IMP.test.TestCase):
    """tests nuisance parameters"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)

    def test_Setup1(self):
        si = Nuisance.setup_particle(IMP.Particle(self.m))
        self.assertAlmostEqual(float(si.get_nuisance()),1.0, delta=1e-6)
        self.assertTrue(math.isinf(-si.get_lower()))
        self.assertTrue(math.isinf(si.get_upper()))

    def test_Setup2(self):
        si = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        si.set_lower(0.1)
        si.set_upper(10)
        self.assertAlmostEqual(float(si.get_nuisance()),2.0, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.1, delta=1e-6)
        self.assertAlmostEqual(float(si.get_upper()),10.0, delta=1e-6)

    def test_Set(self):
        "set returns nothing"
        self.assertEqual(self.sigma.set_nuisance(2),None)

    def test_deriv(self):
        "test setting/getting derivative"
        self.sigma.add_to_nuisance_derivative(123,IMP.DerivativeAccumulator())
        self.assertAlmostEqual(self.sigma.get_nuisance_derivative(),
                123.0,delta=0.01)

    def test_String(self):
        "a parameter cannot take other things than numbers as input"
        self.assertRaises(TypeError, self.sigma.set_nuisance,"a")
        self.assertRaises(TypeError, self.sigma.set_nuisance,(1,2))
        self.assertRaises(TypeError, self.sigma.set_nuisance,[1,2])

    def test_GetSet(self):
        "tests get and set (sanity checks)"
        for si in range(-100,100):
            self.sigma.set_nuisance(si)
            self.assertAlmostEqual(self.sigma.get_nuisance(),si, delta=1e-6)

    def test_GetSet2(self):
        "tests get and set (border check)"
        nuisance = Nuisance.setup_particle(IMP.Particle(self.m), 50.0)
        nuisance.set_lower(10)
        nuisance.set_upper(80)
        for si in range(1,100):
            nuisance.set_nuisance(si)
            if si < 10:
                est = 10
            elif si > 80:
                est = 80
            else:
                est = si
            self.assertAlmostEqual(nuisance.get_nuisance(), est, delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()




        
        
        




        

