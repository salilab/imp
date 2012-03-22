#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import *

#unit testing framework
import IMP.test

class MockFunc:
    def __init__(self, setval, evaluate, evalargs=(1,), update=None):
        self.__set = setval
        self.__eval = evaluate
        self.__update = update
        self.__evalargs = evalargs

    def set_evalargs(self, evalargs):
        self.__evalargs = evalargs

    def __call__(self, value):
        self.__set(value)
        if self.__update:
            self.__update()
        return self.__eval(*self.__evalargs)

class TestGaussianProcessInterpolationNumerically(IMP.test.TestCase):
    """test of the GPI with more data points, using numerical derivative
    estimation.
    """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.TERSE)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        data=open(self.get_input_file_name('lyzexp_gpir.dat')).readlines()
        data=[map(float,d.split()) for d in data]
        self.q=[[i[0]] for i in data]
        self.I=[i[1] for i in data]
        self.err=[i[2] for i in data]
        self.N=10
        self.G = Scale.setup_particle(IMP.Particle(self.m), 3.0)
        self.G.set_nuisance_is_optimized(True)
        self.Rg = Scale.setup_particle(IMP.Particle(self.m),  10.0)
        self.Rg.set_nuisance_is_optimized(True)
        #put d=15 so we don't use the porod region
        self.d = Scale.setup_particle(IMP.Particle(self.m),  15.0)
        self.d.set_nuisance_is_optimized(False)
        self.s = Scale.setup_particle(IMP.Particle(self.m),  0.0)
        self.s.set_nuisance_is_optimized(False)
        self.mean = GeneralizedGuinierPorodFunction(
                self.G,self.Rg,self.d,self.s)
        self.tau = Switching.setup_particle(IMP.Particle(self.m), 1.0)
        self.tau.set_nuisance_is_optimized(True)
        self.lam = Scale.setup_particle(IMP.Particle(self.m), 1.)
        self.lam.set_nuisance_is_optimized(True)
        self.sig = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.sig.set_nuisance_is_optimized(True)
        self.cov = Covariance1DFunction(self.tau, self.lam, 2.0)
        self.gpi = IMP.isd.GaussianProcessInterpolation(self.q, self.I,
                self.err, self.N, self.mean, self.cov, self.sig)
        self.particles=[self.G,self.Rg,self.d,self.s,self.sig,self.tau,self.lam]

    def testDerivNumericG(self):
        """
        test the derivatives of the gpi numerically for G
        """
        pnum=0
        values=linspace(1,10)
        pos=0.1
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance,
                self.gpi.get_posterior_covariance,
                ([pos],[pos]))
        for val in values:
            particle.set_nuisance(val)
            ene=self.gpi.get_posterior_covariance([pos],[pos])
            observed = self.gpi.get_posterior_covariance_derivative([pos],
                    False)[pnum]
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericRg(self):
        """
        test the derivatives of the gpi numerically for Rg
        """
        pnum=1
        values=linspace(1,10)
        pos=0.1
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance,
                self.gpi.get_posterior_covariance,
                ([pos],[pos]))
        for val in values:
            particle.set_nuisance(val)
            ene=self.gpi.get_posterior_covariance([pos],[pos])
            observed = self.gpi.get_posterior_covariance_derivative([pos],
                    False)[pnum]
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericTau(self):
        """
        test the derivatives of the gpi numerically for Tau
        """
        pnum=5
        values=linspace(1,10)
        pos=0.1
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance,
                self.gpi.get_posterior_covariance,
                ([pos],[pos]))
        for val in values:
            particle.set_nuisance(val)
            ene=self.gpi.get_posterior_covariance([pos],[pos])
            observed = self.gpi.get_posterior_covariance_derivative([pos],
                    False)[pnum-2]
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            print "pyy",val,observed,expected,ene
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericSigma(self):
        """
        test the derivatives of the gpi numerically for Sigma
        """
        pnum=4
        values=linspace(1,10)
        pos=0.1
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance,
                self.gpi.get_posterior_covariance,
                ([pos],[pos]))
        for val in values:
            particle.set_nuisance(val)
            ene=self.gpi.get_posterior_covariance([pos],[pos])
            observed = self.gpi.get_posterior_covariance_derivative([pos],
                    False)[pnum-2]
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            print "pyy",val,observed,expected,ene
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericLambda(self):
        """
        test the derivatives of the gpi numerically for Lambda
        """
        pnum=6
        values=linspace(.1,1)
        pos=0.1
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance,
                self.gpi.get_posterior_covariance,
                ([pos],[pos]))
        for val in values:
            particle.set_nuisance(val)
            ene=self.gpi.get_posterior_covariance([pos],[pos])
            observed = self.gpi.get_posterior_covariance_derivative([pos],
                    False)[pnum-2]
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            print "pyy",val,observed,expected,ene
            self.assertAlmostEqual(expected,observed,delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
