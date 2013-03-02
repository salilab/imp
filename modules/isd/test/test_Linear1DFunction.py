#!/usr/bin/env python

#general imports
from numpy import *


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
    """ test of a*x + b function """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.alpha = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.beta = Nuisance.setup_particle(IMP.Particle(self.m),  1.0)
        self.mean = Linear1DFunction(self.alpha,self.beta)
        self.DA=IMP.DerivativeAccumulator()

    def shuffle_particle_values(self):
        particles = [(self.alpha, -100, 100),
                     (self.beta, -100, 100)]
        #number of shuffled values
        for i in xrange(random.randint(0,len(particles))):
            #which particle
            p,imin,imax = particles.pop(random.randint(0,len(particles)))
            p.set_nuisance(random.uniform(imin, imax))
        self.mean.update()

    def test_has_changed(self):
        for p in self.alpha, self.beta:
            p.set_nuisance(p.get_nuisance()+1)
            self.assertTrue(self.mean.has_changed())
            self.mean.update()
            self.assertFalse(self.mean.has_changed())

    def testValue(self):
        """
        test the value of the function by shuffling all particles
        """
        skipped = 0
        for rep in xrange(10):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos = random.uniform(-10,10)
                observed = self.mean([pos])[0]
                a = self.alpha.get_nuisance()
                b = self.beta.get_nuisance()
                expected=a*pos+b
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
        if skipped > 10:
            self.fail("too many NANs")

    def testDerivOne(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on one point at a time
        """
        skipped = 0
        for rep in xrange(10):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos = random.uniform(-10,10)
                self.mean.add_to_derivatives([pos],self.DA)
                #alpha
                expected=pos
                observed = self.alpha.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #beta
                expected=1
                observed = self.beta.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                self.alpha.add_to_nuisance_derivative(
                        -self.alpha.get_nuisance_derivative(),self.DA)
                self.beta.add_to_nuisance_derivative(-self.beta.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

    def testDerivNumericAlpha(self):
        """
        test the derivatives of the function numerically for alpha
        """
        pos = random.uniform(-10,10)
        AlphaFunc = MockFunc(self.alpha.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for alpha in xrange(-10,10):
            self.alpha.set_nuisance(alpha)
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],False)[0][0]
            expected = IMP.test.numerical_derivative(AlphaFunc, alpha, 0.01)
            if observed != 0:
                self.assertAlmostEqual(expected/observed,1,delta=1e-3)
            else:
                self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericBeta(self):
        """
        test the derivatives of the function numerically for beta
        """
        pos = random.uniform(-10,10)
        BetaFunc = MockFunc(self.beta.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for beta in xrange(-10,10):
            self.beta.set_nuisance(beta)
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],False)[0][1]
            expected = IMP.test.numerical_derivative(BetaFunc, beta, 0.01)
            if observed != 0:
                self.assertAlmostEqual(expected/observed,1,delta=1e-3)
            else:
                self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivMult(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on multiple points at a time
        """
        for rep in xrange(10):
            self.shuffle_particle_values()
            expecteda=0
            expectedb=0
            for i in xrange(10):
                pos = random.uniform(-10,10)
                self.mean.add_to_derivatives([pos],self.DA)
                #alpha
                expecteda += pos
                #beta
                expectedb += 1
            observeda = self.alpha.get_nuisance_derivative()
            if expecteda != 0:
                self.assertAlmostEqual(observeda/expecteda
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observeda,expecteda
                    ,delta=0.001)
            observedb = self.beta.get_nuisance_derivative()
            if expectedb != 0:
                self.assertAlmostEqual(observedb/expectedb
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observedb,expectedb
                    ,delta=0.001)
            self.alpha.add_to_nuisance_derivative(-self.alpha.get_nuisance_derivative(),self.DA)
            self.beta.add_to_nuisance_derivative(-self.beta.get_nuisance_derivative(),self.DA)

    def testValues(self):
        """
        tests if we can get multiple values at once
        """
        for rep in xrange(10):
            self.shuffle_particle_values()
            data = random.uniform(-10,10,random.randint(100))
            expected = [self.mean([i]) for i in data]
            observed = self.mean([[i] for i in data],True)
            self.assertEqual(observed,expected)

    def testGetDerivativeMatrix(self):
        for rep in xrange(3):
            self.shuffle_particle_values()
            xlist = random.uniform(-10,10,random.randint(1,100))
            data = self.mean.get_derivative_matrix([[i] for  i in xlist], True)
            self.assertEqual(len(data), len(xlist))
            self.assertEqual(len(data[0]), 2)
            for i,j in zip(data,xlist):
                self.assertAlmostEqual(i[0],j, delta=1e-5)
                self.assertAlmostEqual(i[1],1, delta=1e-5)

    def testAddToParticleDerivative(self):
        for i in xrange(10):
            val = random.uniform(-10,10)
            self.mean.add_to_particle_derivative(0, val, self.DA)
            self.assertAlmostEqual(self.alpha.get_nuisance_derivative(), val)
            self.assertAlmostEqual(self.beta.get_nuisance_derivative(), 0.0)
            self.alpha.add_to_nuisance_derivative(
                    -self.alpha.get_nuisance_derivative(),self.DA)
            val = random.uniform(-10,10)
            self.mean.add_to_particle_derivative(1, val, self.DA)
            self.assertAlmostEqual(self.alpha.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.beta.get_nuisance_derivative(), val)
            self.beta.add_to_nuisance_derivative(
                    -self.beta.get_nuisance_derivative(),self.DA)

if __name__ == '__main__':
    IMP.test.main()
