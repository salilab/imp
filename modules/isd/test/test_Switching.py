#!/usr/bin/env python

import math
#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Switching,Nuisance

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    """tests switching parameters"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Switching.setup_particle(IMP.Particle(self.m), .5)

    def test_Setup1(self):
        "Test Switching setup without upper/lower"
        si = Switching.setup_particle(IMP.Particle(self.m))
        self.assertAlmostEqual(float(si.get_switching()),.5, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.0, delta=1e-6)
        self.assertAlmostEqual(float(si.get_upper()),1.0, delta=1e-6)

    def test_Setup2(self):
        "Test Switching setup with upper/lower"
        si = Switching.setup_particle(IMP.Particle(self.m), 0.2)
        si.set_lower(0.1)
        si.set_upper(0.8)
        self.assertAlmostEqual(float(si.get_switching()),.2, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.1, delta=1e-6)
        self.assertAlmostEqual(float(si.get_upper()),0.8, delta=1e-6)

    def test_Switching(self):
        "Test that a Switching can be converted to a Nuisance"
        n=Switching.setup_particle(IMP.Particle(self.m))
        n.set_lower(-10)
        n.set_upper(10)
        self.assertTrue(Nuisance.particle_is_instance(n.get_particle()))
        self.assertFalse(Switching.particle_is_instance(n.get_particle()))

    def test_Nuisance(self):
        "Test that a Nuisance can be converted to a Switching"
        n=Nuisance.setup_particle(IMP.Particle(self.m),0.5)
        n.set_lower(0)
        n.set_upper(1)
        self.assertTrue(Switching.particle_is_instance(n.get_particle()))

    def test_Set(self):
        "Switching set returns nothing"
        self.assertEqual(self.sigma.set_switching(0.1),None)

    def test_deriv(self):
        "Test setting/getting Switching derivative"
        self.sigma.add_to_switching_derivative(123,IMP.DerivativeAccumulator())
        self.assertAlmostEqual(self.sigma.get_switching_derivative(),
                123.0,delta=0.01)

    def test_String(self):
        "Switching parameter cannot take other things than numbers as input"
        self.assertRaises(TypeError, self.sigma.set_switching,"a")
        self.assertRaises(TypeError, self.sigma.set_switching,(1,2))
        self.assertRaises(TypeError, self.sigma.set_switching,[1,2])

    def test_GetSet(self):
        "Test Switching get and set (sanity checks)"
        for si in range(1,100):
            self.sigma.set_switching(si/100.)
            self.assertAlmostEqual(self.sigma.get_switching(),si/100., delta=1e-6)

    def test_GetSet2(self):
        "Test Switching get and set (border check)"
        switching = Switching.setup_particle(IMP.Particle(self.m))
        for i in range(-10,20):
            si = i/10.
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
