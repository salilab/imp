#!/usr/bin/env python

import math
#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Switching

#unit testing framework
import IMP.test

class TestSwitchingParam(IMP.test.TestCase):
    """tests switching parameters"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Switching.setup_particle(IMP.Particle(self.m))

    def test_Bounds(self):
        si = Switching.setup_particle(IMP.Particle(self.m))
        self.assertAlmostEqual(float(si.get_switching()),0.5, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.0, delta=1e-6)
        self.assertAlmostEqual(float(si.get_upper()),1.0, delta=1e-6)

    def test_Set(self):
        "set returns nothing"
        self.assertEqual(self.sigma.set_switching(0.1),None)

    def test_deriv(self):
        "test setting/getting derivative"
        self.sigma.add_to_switching_derivative(123,IMP.DerivativeAccumulator())
        self.assertAlmostEqual(self.sigma.get_switching_derivative(),
                123.0,delta=0.01)

    def test_String(self):
        "a parameter cannot take other things than numbers as input"
        self.assertRaises(TypeError, self.sigma.set_switching,"a")
        self.assertRaises(TypeError, self.sigma.set_switching,(1,2))
        self.assertRaises(TypeError, self.sigma.set_switching,[1,2])

    def test_GetSet(self):
        "tests get and set (sanity checks)"
        for i in range(1,100):
            si = i/100.
            self.sigma.set_switching(si)
            self.assertAlmostEqual(self.sigma.get_switching(),si, delta=1e-6)

    def test_GetSet2(self):
        "tests get and set (border check)"
        switching = Switching.setup_particle(IMP.Particle(self.m))
        for i in range(1,100):
            si = 2*(i/100.)-1
            switching.set_switching(si)
            if si < 0:
                est = 0
            elif si > 1:
                est = 1
            else:
                est = si
            self.assertAlmostEqual(switching.get_switching(), est, delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()

