#!/usr/bin/env python

#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Scale,Nuisance

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    """tests scale parameters"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 1.0)

    def test_Setup1(self):
        "Test scale setup without lower/upper"
        si = Scale.setup_particle(IMP.Particle(self.m))
        self.assertAlmostEqual(float(si.get_scale()),1.0, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.0, delta=1e-6)
        self.assertFalse(si.has_upper())
        self.assertEqual(si.get_upper(), 1e3000) # 1e3000 ~= inf

    def test_Setup2(self):
        "Test scale setup with lower/upper"
        si = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        si.set_lower(0.1)
        si.set_upper(10)
        self.assertAlmostEqual(float(si.get_scale()),2.0, delta=1e-6)
        self.assertAlmostEqual(float(si.get_lower()),0.1, delta=1e-6)
        self.assertAlmostEqual(float(si.get_upper()),10.0, delta=1e-6)

    def test_Nuisance(self):
        "Test that a Nuisance can be converted to a Scale"
        n=Nuisance.setup_particle(IMP.Particle(self.m),3.0)
        n.set_lower(0)
        self.assertTrue(Scale.particle_is_instance(n.get_particle()))

    def test_Set(self):
        "Scale set returns nothing"
        self.assertEqual(self.sigma.set_scale(2),None)

    def test_deriv(self):
        "Test setting/getting Scale derivative"
        self.sigma.add_to_scale_derivative(123,IMP.DerivativeAccumulator())
        self.assertAlmostEqual(self.sigma.get_scale_derivative(),
                123.0,delta=0.01)

    def test_String(self):
        "Scale parameter cannot take other things than numbers as input"
        self.assertRaises(TypeError, self.sigma.set_scale,"a")
        self.assertRaises(TypeError, self.sigma.set_scale,(1,2))
        self.assertRaises(TypeError, self.sigma.set_scale,[1,2])

    def test_GetSet(self):
        "Test Scale get and set (sanity checks)"
        for si in range(1,100):
            self.sigma.set_scale(si)
            self.assertAlmostEqual(self.sigma.get_scale(),si, delta=1e-6)

    def test_GetSet2(self):
        "Test Scale get and set (border check)"
        scale = Scale.setup_particle(IMP.Particle(self.m), 50.0)
        scale.set_lower(10)
        scale.set_upper(80)
        for si in range(1,100):
            scale.set_scale(si)
            if si < 10:
                est = 10
            elif si > 80:
                est = 80
            else:
                est = si
            self.assertAlmostEqual(scale.get_scale(), est, delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
