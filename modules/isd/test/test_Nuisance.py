#!/usr/bin/env python

#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Nuisance

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    """tests nuisance parameters"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)

    def test_Setup1(self):
        "Test nuisance parameter setup without lower/upper"
        si = Nuisance.setup_particle(IMP.Particle(self.m))
        self.assertAlmostEqual(float(si.get_nuisance()),1.0, delta=1e-6)
        self.assertEqual(si.get_lower(), -1e3000) # -1e3000 ~= -inf
        self.assertEqual(si.get_upper(), 1e3000) # 1e3000 ~= inf

    def test_Setup2(self):
        "Test nuisance parameter setup with lower/upper"
        si = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        si.set_lower(0.1)
        si.set_upper(10)
        self.assertAlmostEqual(float(si.get_nuisance()),2.0, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.1, delta=1e-6)
        self.assertAlmostEqual(float(si.get_upper()),10.0, delta=1e-6)

    def test_Set(self):
        "Test set_nuisance returns nothing"
        self.assertEqual(self.sigma.set_nuisance(2),None)

    def test_deriv(self):
        "Test setting/getting nuisance derivative"
        self.sigma.add_to_nuisance_derivative(123,IMP.DerivativeAccumulator())
        self.assertAlmostEqual(self.sigma.get_nuisance_derivative(),
                123.0,delta=0.01)

    def test_String(self):
        "Nuisance parameter cannot take things other than numbers as input"
        self.assertRaises(TypeError, self.sigma.set_nuisance,"a")
        self.assertRaises(TypeError, self.sigma.set_nuisance,(1,2))
        self.assertRaises(TypeError, self.sigma.set_nuisance,[1,2])

    def test_GetSet(self):
        "Test nuisance get and set (sanity checks)"
        for si in range(-100,100):
            self.sigma.set_nuisance(si)
            self.assertAlmostEqual(self.sigma.get_nuisance(),si, delta=1e-6)

    def test_GetSet2(self):
        "Test nuisance get and set (border check)"
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

    def test_GetSet_Particle(self):
        "Test nuisance get and set (border check) with particles"
        nuisance = Nuisance.setup_particle(IMP.Particle(self.m), 50.0)
        lower = Nuisance.setup_particle(IMP.Particle(self.m), 10.0)
        upper = Nuisance.setup_particle(IMP.Particle(self.m), 80.0)
        nuisance.set_lower(lower)
        nuisance.set_upper(upper)
        for si in range(1,100):
            nuisance.set_nuisance(si)
            if si < 10:
                est = 10
            elif si > 80:
                est = 80
            else:
                est = si
            self.assertAlmostEqual(nuisance.get_nuisance(), est, delta=1e-6)

    def test_GetSet_Both(self):
        "Test nuisance get/set (border check) with both particles and floats"
        nuisance = Nuisance.setup_particle(IMP.Particle(self.m), 50.0)
        lower = Nuisance.setup_particle(IMP.Particle(self.m), 10.0)
        upper = Nuisance.setup_particle(IMP.Particle(self.m), 90.0)
        nuisance.set_lower(1.0)
        nuisance.set_lower(lower)
        nuisance.set_upper(upper)
        nuisance.set_upper(80.0)
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
