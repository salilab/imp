#!/usr/bin/env python

#general imports
from numpy import *
from random import *


#imp general
import IMP
import IMP.gsl
import IMP.core

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
    gaussian prior covariances
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
        self.G.set_nuisance_is_optimized(True)
        self.Rg = Scale.setup_particle(IMP.Particle(self.m),  10.0)
        self.Rg.set_nuisance_is_optimized(True)
        self.d = Scale.setup_particle(IMP.Particle(self.m),  4.0)
        self.d.set_nuisance_is_optimized(False)
        self.s = Scale.setup_particle(IMP.Particle(self.m),  0.0)
        self.s.set_nuisance_is_optimized(False)
        self.A = Scale.setup_particle(IMP.Particle(self.m),  1.0)
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
        particles = [(self.G, 3, 10),
                     (self.Rg, 10, 30),
                     (self.d, 1, 4),
                     (self.tau, 0.001, 10),
                     (self.lam, 0.1, 10),
                     (self.sig, 0.1, 10)]
        #number of shuffled values
        for i in xrange(randint(0,5)):
            #which particle
            p,imin,imax = particles.pop(randint(0,len(particles)-1))
            p.set_nuisance(uniform(imin, imax))


    def testDerivNumericG(self):
        """
        test the derivatives of the gpi numerically for G
        """
        pnum=0
        values=linspace(1,10)
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericRg(self):
        """
        test the derivatives of the gpi numerically for Rg
        """
        pnum=1
        values=linspace(1,10)
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-1)

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


    def testHessianNumericGG(self):
        """
        test the Hessian of the function numerically wrt G and G
        """
        pa=0
        pb=0
        values=range(1,5)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa][pb]
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericGRg(self):
        """
        test the Hessian of the function numerically wrt G and Rg
        """
        pa=1
        pb=0
        values=linspace(1,10)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa][pb]
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericRgRg(self):
        """
        test the Hessian of the function numerically wrt Rg and Rg
        """
        pa=1
        pb=1
        values=linspace(1,10)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa][pb]
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericRgG(self):
        """
        test the Hessian of the function numerically wrt Rg and G
        """
        pa=1
        pb=0
        values=linspace(1,10)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa][pb]
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)


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
            observed = self.gpr.get_hessian(False)[pa-2][pb-2] #s and d not opt
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
            observed = self.gpr.get_hessian(False)[pa-2][pb-2] #s and d not opt
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
            observed = self.gpr.get_hessian(False)[pa-2][pb-2] #s and d not opt
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
            observed = self.gpr.get_hessian(False)[pa-2][pb-2] #s and d not opt
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
            observed = self.gpr.get_hessian(False)[pa-2][pb-2] #s and d not opt
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
            observed = self.gpr.get_hessian(False)[pa-2][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)


    def testHessianNumericGSigma(self):
        """
        test the Hessian of the function numerically wrt G and Sigma
        """
        pa=0
        pb=4
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericRgSigma(self):
        """
        test the Hessian of the function numerically wrt Rg and Sigma
        """
        pa=1
        pb=4
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericGTau(self):
        """
        test the Hessian of the function numerically wrt G and Tau
        """
        pa=0
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
            observed = self.gpr.get_hessian(False)[pa][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericRgTau(self):
        """
        test the Hessian of the function numerically wrt Rg and Tau
        """
        pa=1
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
            observed = self.gpr.get_hessian(False)[pa][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericGLambda(self):
        """
        test the Hessian of the function numerically wrt G and Lambda
        """
        pa=0
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
            observed = self.gpr.get_hessian(False)[pa][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericRgLambda(self):
        """
        test the Hessian of the function numerically wrt Rg and Lambda
        """
        pa=1
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
            observed = self.gpr.get_hessian(False)[pa][pb-2] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericLambdaRg(self):
        """
        test the Hessian of the function numerically wrt Rg and Lambda
        """
        pa=6
        pb=1
        values=linspace(.1,.9)
        ppa=self.particles[pa]
        ppb=self.particles[pb]
        PFunc = MockFunc(ppb.set_nuisance,
                lambda a:ppa.get_nuisance_derivative(), None,
                update=lambda: self.m.evaluate(True))
        for val in values:
            ppb.set_nuisance(val)
            #IMP.base.set_log_level(IMP.base.TERSE)
            observed = self.gpr.get_hessian(False)[pa-2][pb] #s and d not opt
            #IMP.base.set_log_level(0)
            expected = IMP.test.numerical_derivative(PFunc, val, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianSymmetry(self):
        #self.shuffle_particle_values()
        #gen list of all possible combinations
        combinations=[[]]
        for np in xrange(len(self.particles)):
            tmp = [[True]+i[:] for i in combinations]
            tmp += [[False]+i[:] for i in combinations]
            combinations = [i[:] for i in tmp]
        #do each combinations 3 times
        for comb in combinations:
            np=0
            #print comb
            for c,p in zip(comb,self.particles):
                p.set_nuisance_is_optimized(comb)
                np += 1
            for rep in xrange(3):
                self.shuffle_particle_values()
                Hessian = array(self.gpr.get_hessian(False))
                self.assertEqual(Hessian.shape, (np,np))
                if hasattr(linalg, 'matrix_rank'):
                    self.assertEqual(linalg.matrix_rank(Hessian), np)
                #try:
                #    self.assertEqual(linalg.matrix_rank(Hessian), np)
                #except:
                #    print Hessian
                #    numrow=[(Hessian[i] == zeros(np)).all() for i in
                #            xrange(np)].index(True)
                #    particlenum = 0
                #    nopt=0
                #    for c in zip(comb):
                #        if c:
                #            if numrow == nopt:
                #               break
                #            nopt+=1
                #        particlenum += 1
                #    print numrow,particlenum,self.particles[particlenum].get_nuisance()
                #    raise
                for i in xrange(np):
                    for j in xrange(i,np):
                        self.assertAlmostEqual(Hessian[i,j],Hessian[j,i],
                                                delta=1e-7)

    def testScoreState(self):
        """test if the scorestate works """
        self.G.set_nuisance(0.)
        self.A.set_nuisance(0.)
        self.G.set_nuisance_is_optimized(False)
        self.Rg.set_nuisance_is_optimized(False)
        self.d.set_nuisance_is_optimized(False)
        self.s.set_nuisance_is_optimized(False)
        self.A.set_nuisance_is_optimized(True)
        self.tau.set_nuisance_is_optimized(False)
        self.lam.set_nuisance_is_optimized(False)
        self.sig.set_nuisance_is_optimized(False)
        target = 14.7
        #energy should lower if I move A closer to its minimum
        #model.evaluate()
        ene = self.m.evaluate(False)
        self.A.set_nuisance(target/2.)
        newene = self.m.evaluate(False)
        self.assertTrue(newene < ene)
        #gpr.evaluate()
        self.A.set_nuisance(0.)
        ene = self.gpr.evaluate(False)
        self.A.set_nuisance(target/2.)
        newene = self.gpr.evaluate(False)
        self.assertTrue(newene < ene)
        # an optimizer should be able to find the minimum
        cg = IMP.core.ConjugateGradients(self.m)
        cg.optimize(100)
        self.assertAlmostEqual(self.A.get_nuisance(), target, delta=1e-1)
        # an optimizer should be able to find the minimum
        cg = IMP.gsl.ConjugateGradients(self.m)
        cg.optimize(100)
        self.assertAlmostEqual(self.A.get_nuisance(), target, delta=1e-1)

if __name__ == '__main__':
    IMP.test.main()
