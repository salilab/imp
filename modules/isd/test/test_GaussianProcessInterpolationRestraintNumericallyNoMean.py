#!/usr/bin/env python

#general imports
from numpy import *
from random import *


#imp general
import IMP

#our project
from IMP.isd import *

#unit testing framework
import IMP.test

class MockFunc:
    def __init__(self, setval, evaluate, evalargs=1, update=None):
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
        return self.__eval(self.__evalargs)

class Tests(IMP.test.TestCase):
    """test of the GPI restraint with two data points, linear prior mean and
    gaussian prior covariances. Mean function is not optimized.
    """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.base.TERSE)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        data=open(self.get_input_file_name('lyzexp_gpir.dat')).readlines()
        data=[map(float,d.split()) for d in data]
        self.q=[[i[0]] for i in data]
        self.I=[i[1] for i in data]
        self.err=[i[2] for i in data]
        self.N=10
        self.G = Scale.setup_particle(IMP.Particle(self.m), 3.0)
        self.G.set_nuisance_is_optimized(False)
        self.Rg = Scale.setup_particle(IMP.Particle(self.m),  10.0)
        self.Rg.set_nuisance_is_optimized(False)
        #put d=15 so we don't use the porod region
        self.d = Scale.setup_particle(IMP.Particle(self.m),  15.0)
        self.d.set_nuisance_is_optimized(False)
        self.s = Scale.setup_particle(IMP.Particle(self.m),  0.0)
        self.s.set_nuisance_is_optimized(False)
        self.A = Scale.setup_particle(IMP.Particle(self.m),  0.0)
        self.A.set_nuisance_is_optimized(False)
        self.mean = GeneralizedGuinierPorodFunction(
                self.G,self.Rg,self.d,self.s, self.A)
        self.tau = Switching.setup_particle(IMP.Particle(self.m), 1.0)
        self.tau.set_nuisance_is_optimized(True)
        self.lam = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.lam.set_nuisance_is_optimized(True)
        self.sig = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.sig.set_nuisance_is_optimized(True)
        self.cov = Covariance1DFunction(self.tau, self.lam, 2.0)
        self.gpi = IMP.isd.GaussianProcessInterpolation(self.q, self.I,
                self.err, self.N, self.mean, self.cov, self.sig)
        self.gpr = IMP.isd.GaussianProcessInterpolationRestraint(self.gpi)
        self.m.add_restraint(self.gpr)
        self.particles=[self.G,self.Rg,self.d,self.s,self.sig,self.tau,self.lam]

    def shuffle_particle_values(self):
        particles = [(self.alpha, -10, 10),
                     (self.beta, -10, 10),
                     (self.tau, 0.001, 10),
                     (self.lam, 0.1, 10),
                     (self.sig, 0.1, 10)]
        #number of shuffled values
        for i in xrange(randint(0,5)):
            #which particle
            p,imin,imax = particles.pop(randint(0,len(particles)-1))
            p.set_nuisance(uniform(imin, imax))


    def testDerivNumericSigma(self):
        """
        test the derivatives of the gpi numerically for Sigma
        """
        pnum=4
        values=range(1,10)
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, .1)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testDerivNumericTau(self):
        """
        test the derivatives of the gpi numerically for Tau
        """
        pnum=5
        values=linspace(.1,.9)
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, .01)
            self.assertAlmostEqual(expected,observed,delta=5e-2)

    def testDerivNumericLambda(self):
        """
        test the derivatives of the gpi numerically for Lambda
        """
        pnum=6
        values=linspace(.3,2)
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, .02)
            self.assertAlmostEqual(expected,observed,delta=1e-2)


    def testHessianNumericSigmaSigma(self):
        """
        test the Hessian of the function numerically wrt Sigma and Sigma
        """
        pa=4
        pb=4
        values=range(1,3)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-4][pb-4] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericSigmaTau(self):
        """
        test the Hessian of the function numerically wrt Sigma and Tau
        """
        pa=4
        pb=5
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-4][pb-4] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericSigmaLambda(self):
        """
        test the Hessian of the function numerically wrt Sigma and Lambda
        """
        pa=4
        pb=6
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-4][pb-4] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericTauTau(self):
        """
        test the Hessian of the function numerically wrt Tau and Tau
        """
        pa=5
        pb=5
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-4][pb-4] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericTauLambda(self):
        """
        test the Hessian of the function numerically wrt Tau and Lambda
        """
        pa=5
        pb=6
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-4][pb-4] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericLambdaLambda(self):
        """
        test the Hessian of the function numerically wrt Lambda and Lambda
        """
        pa=6
        pb=6
        values=linspace(1,10)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-4][pb-4] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
