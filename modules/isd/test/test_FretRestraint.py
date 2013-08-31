#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform

#imp general
import IMP

import math

#our project
from IMP.isd import Scale,FretRestraint

#unit testing framework
import IMP.test

class TestFretRestraint(IMP.test.TestCase):
    "simple test cases to check if FretRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.kda =   Scale.setup_particle(IMP.kernel.Particle(self.m), 4.0)
        self.Ida =   Scale.setup_particle(IMP.kernel.Particle(self.m), 3.0)
        self.R0  =   Scale.setup_particle(IMP.kernel.Particle(self.m), 50.0)
        self.sigma = Scale.setup_particle(IMP.kernel.Particle(self.m), 0.005)
        self.Pbl   = Scale.setup_particle(IMP.kernel.Particle(self.m), 0.7)
        self.p0=IMP.core.XYZ.setup_particle(IMP.kernel.Particle(self.m),
            IMP.algebra.Vector3D(50,0,0))
        self.p1=IMP.core.XYZ.setup_particle(IMP.kernel.Particle(self.m),
            IMP.algebra.Vector3D(0,50,0))
        self.p2=IMP.core.XYZ.setup_particle(IMP.kernel.Particle(self.m),
            IMP.algebra.Vector3D(0,0,0))
        self.fexp = 1.5
        self.fret0 = IMP.isd.FretRestraint([self.p0],[self.p1,self.p2],
                self.kda,self.Ida,self.R0,self.sigma,self.Pbl,self.fexp)
        self.fret1 = IMP.isd.FretRestraint([self.p0,self.p1],[self.p2],
                self.kda,self.Ida,self.R0,self.sigma,self.Pbl,self.fexp)

    def reset(self):
        self.p0.set_coordinates(IMP.algebra.Vector3D(50,0,0))
        self.p1.set_coordinates(IMP.algebra.Vector3D(0,50,0))
        self.p2.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        self.kda.set_scale(4.0)
        self.Ida.set_scale(3.0)
        self.R0.set_scale(50.0)
        self.sigma.set_scale(0.005)
        self.Pbl.set_scale(0.7)
        self.fexp=1.5
        self.fret0.set_experimental_value(self.fexp)
        self.fret1.set_experimental_value(self.fexp)

    def getFM0(self):
        Ida = self.Ida.get_scale()
        kda = self.kda.get_scale()
        Pbl = self.Pbl.get_scale()
        dist01 = IMP.core.get_distance(self.p0,self.p1)
        dist02 = IMP.core.get_distance(self.p0,self.p2)
        F01 = (self.R0.get_scale()/dist01)**6
        F02 = (self.R0.get_scale()/dist02)**6
        sumFi_aa = ( 1.0 / ( 1.0 + ( F01 + F02 ) ) )
        sumFi_dd = ( 1.0 / ( 1.0 + ( 0.0 + 0.0 ) ) )
        sumFi_ad = ( 1.0 / ( 1.0 + ( F01 + 0.0 ) ) )
        sumFi_da = ( 1.0 / ( 1.0 + ( 0.0 + F02 ) ) )
        p_aa = Pbl * Pbl
        p_dd = (1.0-Pbl) * (1.0-Pbl)
        p_ad = Pbl * (1.0-Pbl)
        p_da = (1.0-Pbl) * Pbl
        sumFi_ave = p_aa*sumFi_aa+p_dd*sumFi_dd+p_ad*sumFi_ad+p_da*sumFi_da
        fretr = (Ida*sumFi_aa+2.0+kda*(1.0-sumFi_aa))/(Ida*sumFi_ave+2.0)
        return fretr

    def getFM1(self):
        Ida = self.Ida.get_scale()
        kda = self.kda.get_scale()
        Pbl = self.Pbl.get_scale()
        dist02 = IMP.core.get_distance(self.p0,self.p2)
        dist12 = IMP.core.get_distance(self.p1,self.p2)
        F02 = (self.R0.get_scale()/dist02)**6
        F12 = (self.R0.get_scale()/dist12)**6
        sumFi_a0 = ( 1.0 / ( 1.0 + F02 ) )
        sumFi_d0 = ( 1.0 / ( 1.0 + 0.0 ) )
        sumFi_a1 = ( 1.0 / ( 1.0 + F12 ) )
        sumFi_d1 = ( 1.0 / ( 1.0 + 0.0 ) )
        p_a = Pbl
        p_d = (1.0-Pbl)
        sumFi = sumFi_a0 + sumFi_a1
        sumFi_ave = p_a*sumFi_a0+p_d*sumFi_d0+p_a*sumFi_a1+p_d*sumFi_d1
        fretr = (Ida*sumFi+1.0+kda*(2.0-sumFi))/(Ida*sumFi_ave+1.0)
        return fretr

    def getP(self,fmod):
        log_eps = math.log(self.fexp/fmod)
        sigma = self.sigma.get_scale()
        prob = math.sqrt(2.0)*sigma/self.fexp/math.pi/(log_eps*log_eps+2.0*sigma*sigma)
        return prob

    def testValueFMDist(self):
        "Test FretRestraint Forward Model by changing distances"
        self.reset()
        for i in xrange(100):
            pos0=[uniform(0.1,100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1=[uniform(0.1,100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            pos2=[uniform(0.1,100) for i in range(3)]
            self.p2.set_coordinates(IMP.algebra.Vector3D(*pos2))
            expected0 = self.getFM0()
            expected1 = self.getFM1()
            self.assertAlmostEqual(self.fret0.get_model_fretr(),
                    expected0,delta=0.01)
            self.assertAlmostEqual(self.fret1.get_model_fretr(),
                    expected1,delta=0.01)

    def testValueFMIda(self):
        "Test FretRestraint Forward Model by changing Ida"
        self.reset()
        for i in xrange(100):
            Ida=uniform(1.0,10.0)
            self.Ida.set_scale(Ida)
            expected0 = self.getFM0()
            expected1 = self.getFM1()
            self.assertAlmostEqual(self.fret0.get_model_fretr(),
                    expected0,delta=0.01)
            self.assertAlmostEqual(self.fret1.get_model_fretr(),
                    expected1,delta=0.01)

    def testValueFMkda(self):
        "Test FretRestraint Forward Model by changing kda"
        self.reset()
        for i in xrange(100):
            kda=uniform(1.0,20.0)
            self.kda.set_scale(kda)
            expected0 = self.getFM0()
            expected1 = self.getFM1()
            self.assertAlmostEqual(self.fret0.get_model_fretr(),
                    expected0,delta=0.01)
            self.assertAlmostEqual(self.fret1.get_model_fretr(),
                    expected1,delta=0.01)

    def testValueFMPbl(self):
        "Test FretRestraint Forward Model by changing Pbl"
        self.reset()
        for i in xrange(100):
            Pbl=uniform(0.6,1.0)
            self.Pbl.set_scale(Pbl)
            expected0 = self.getFM0()
            expected1 = self.getFM1()
            self.assertAlmostEqual(self.fret0.get_model_fretr(),
                    expected0,delta=0.05)
            self.assertAlmostEqual(self.fret1.get_model_fretr(),
                    expected1,delta=0.05)

    def testValueFMR0(self):
        "Test FretRestraint Forward Model by changing R0"
        self.reset()
        for i in xrange(100):
            R0=uniform(30.0,60.0)
            self.R0.set_scale(R0)
            expected0 = self.getFM0()
            expected1 = self.getFM1()
            self.assertAlmostEqual(self.fret0.get_model_fretr(),
                    expected0,delta=0.01)
            self.assertAlmostEqual(self.fret1.get_model_fretr(),
                    expected1,delta=0.01)

    def testValuePfexp(self):
        "Test FretRestraint probability by changing fexp value"
        self.reset()
        fretr0=self.getFM0()
        fretr1=self.getFM1()
        for i in xrange(100):
            self.fexp = uniform(0.0,10.0)
            expected0=self.getP(fretr0)
            expected1=self.getP(fretr1)
            self.fret0.set_experimental_value(self.fexp)
            self.fret1.set_experimental_value(self.fexp)
            self.assertAlmostEqual(self.fret0.get_probability(),
                    expected0,delta=0.001)
            self.assertAlmostEqual(self.fret1.get_probability(),
                    expected1,delta=0.001)

    def testValuePSigma(self):
        "Test FretRestraint probability by changing sigma"
        self.reset()
        fretr0=self.getFM0()
        fretr1=self.getFM1()
        for i in xrange(100):
            sigma=uniform(0.001,0.01)
            self.sigma.set_scale(sigma)
            expected0=self.getP(fretr0)
            expected1=self.getP(fretr1)
            self.assertAlmostEqual(self.fret0.get_probability(),
                    expected0,delta=0.001)
            self.assertAlmostEqual(self.fret1.get_probability(),
                    expected1,delta=0.001)

if __name__ == '__main__':
    IMP.test.main()
