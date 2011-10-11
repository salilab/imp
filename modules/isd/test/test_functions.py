#!/usr/bin/env python

#general imports
from numpy import *


#imp general
import IMP

#our project
from IMP.isd import *

#unit testing framework
import IMP.test

class TestLinear1DFunction(IMP.test.TestCase):
    """ test of a*x + b function """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
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
                self.alpha.add_to_nuisance_derivative(-self.alpha.get_nuisance_derivative(),self.DA)
                self.beta.add_to_nuisance_derivative(-self.beta.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

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
            observed = self.mean([[i] for i in data])
            self.assertEqual(observed,expected)

    def testGetDerivativeMatrix(self):
        for rep in xrange(3):
            self.shuffle_particle_values()
            xlist = random.uniform(-10,10,random.randint(100))
            data = self.mean.get_derivative_matrix([[i] for  i in xlist], True)
            self.assertEqual(len(data), len(xlist))
            self.assertEqual(len(data[0]), 2)
            self.assertEqual([i[0] for i in data], xlist)
            self.assertEqual([i[1] for i in data], [1 for i in data])

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

class TestCovariance1DFunction(IMP.test.TestCase):
    """ test of tau**2 exp( -(q-q')**2/(2 lambda**2) ) + sigma**2 """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        IMP.set_log_level(0)
        self.tau = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.lam = Scale.setup_particle(IMP.Particle(self.m),  1.0)
        self.sig = Scale.setup_particle(IMP.Particle(self.m),  1.0)
        self.cov = Covariance1DFunction(self.tau,self.lam,self.sig)
        self.DA=IMP.DerivativeAccumulator()

    def shuffle_particle_values(self):
        particles = [(self.tau,    0, 10),
                     (self.lam,    0, 10),
                     (self.sig,    0, 10)]
        #number of shuffled values
        for i in xrange(random.randint(0,len(particles))):
            #which particle
            p,imin,imax = particles.pop(random.randint(0,len(particles)))
            p.set_scale(random.uniform(imin, imax))
        self.cov.update()

    def test_has_changed(self):
        for p in self.tau, self.lam, self.sig:
            p.set_nuisance(p.get_nuisance()+1)
            self.assertTrue(self.cov.has_changed())
            self.cov.update()
            self.assertFalse(self.cov.has_changed())

    def testFail(self):
        q=IMP.Particle(self.m)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, self.tau,q,self.lam)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, q,self.tau,self.lam)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, self.tau,self.lam,q)

    def testValue(self):
        """
        test the value of the function by shuffling all particles
        """
        for rep in xrange(100):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos1 = random.uniform(-10,10)
                for j in xrange(10):
                    pos2 = random.uniform(-10,10)
                    observed = self.cov([pos1],[pos2])[0]
                    t = self.tau.get_nuisance()
                    l = self.lam.get_nuisance()
                    s = self.sig.get_nuisance()
                    expected=t**2*exp(-0.5*(abs(pos1-pos2)/l)**2)
                    if abs(pos1-pos2) < 1e-7:
                        expected += s**2
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected ,delta=0.001)
                pos2=pos1
                observed = self.cov([pos1],[pos2])[0]
                t = self.tau.get_nuisance()
                l = self.lam.get_nuisance()
                s = self.sig.get_nuisance()
                expected=t**2*exp(-0.5*(abs(pos1-pos2)/l)**2)
                expected += s**2
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)

    def testDerivOne(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on one point at a time
        """
        skipped = 0
        for rep in xrange(10):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos1 = random.uniform(-10,10)
                for j in xrange(10):
                    pos2 = random.uniform(-10,10)
                    t = self.tau.get_nuisance()
                    l = self.lam.get_nuisance()
                    s = self.sig.get_nuisance()
                    self.cov.add_to_derivatives([pos1],[pos2],self.DA)
                    #tau
                    expected = 2*t*exp(-0.5*(abs(pos1-pos2)/l)**2)
                    observed = self.tau.get_nuisance_derivative()
                    if isnan(expected):
                        skipped += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
                    #lambda
                    expected = t**2*exp(-0.5*(abs(pos1-pos2)/l)**2)*abs(pos1-pos2)**2/l**3
                    observed = self.lam.get_nuisance_derivative()
                    if isnan(expected):
                        skipped += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
                    #sigma
                    if abs(pos1-pos2) < 1e-7:
                        expected = 2*s
                    else:
                        expected = 0
                    observed = self.sig.get_nuisance_derivative()
                    if isnan(expected):
                        skipped += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
                    self.tau.add_to_nuisance_derivative(-self.tau.get_nuisance_derivative(),self.DA)
                    self.lam.add_to_nuisance_derivative(-self.lam.get_nuisance_derivative(),self.DA)
                    self.sig.add_to_nuisance_derivative(-self.sig.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

    def testDerivOneSame(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on one point at a time when pos1=pos2
        """
        skipped = 0
        for rep in xrange(10):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos1 = random.uniform(-10,10)
                pos2 = pos1
                t = self.tau.get_nuisance()
                l = self.lam.get_nuisance()
                s = self.sig.get_nuisance()
                self.cov.add_to_derivatives([pos1],[pos2],self.DA)
                #tau
                expected = 2*t*exp(-0.5*(abs(pos1-pos2)/l)**2)
                observed = self.tau.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #lambda
                expected = t**2*exp(-0.5*(abs(pos1-pos2)/l)**2)*abs(pos1-pos2)**2/l**3
                observed = self.lam.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #sigma
                if abs(pos1-pos2) < 1e-7:
                    expected = 2*s
                else:
                    expected = 0
                observed = self.sig.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                self.tau.add_to_nuisance_derivative(-self.tau.get_nuisance_derivative(),self.DA)
                self.lam.add_to_nuisance_derivative(-self.lam.get_nuisance_derivative(),self.DA)
                self.sig.add_to_nuisance_derivative(-self.sig.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

class TestReparametrizedCovariance1DFunction(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        IMP.set_log_level(0)
        self.theta = Switching.setup_particle(IMP.Particle(self.m), 0.5)
        self.lam = Scale.setup_particle(IMP.Particle(self.m),  1.0)
        self.sig = Scale.setup_particle(IMP.Particle(self.m),  1.0)
        self.cov = ReparametrizedCovariance1DFunction(
                self.sig,self.lam,self.theta)
        self.DA=IMP.DerivativeAccumulator()

    def shuffle_particle_values(self):
        particles = [(self.theta,    0, 1),
                     (self.lam,    1, 10),
                     (self.sig,    0, 10)]
        #number of shuffled values
        for i in xrange(random.randint(0,len(particles))):
            #which particle
            p,imin,imax = particles.pop(random.randint(0,len(particles)))
            p.set_nuisance(random.uniform(imin, imax))
        self.cov.update()

    def test_has_changed(self):
        for p in self.theta, self.lam, self.sig:
            p.set_nuisance(p.get_nuisance()+1)
            self.assertTrue(self.cov.has_changed())
            self.cov.update()
            self.assertFalse(self.cov.has_changed())

    def testFail(self):
        q=IMP.Particle(self.m)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, self.theta,q,self.lam)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, q,self.theta,self.lam)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, self.theta,self.lam,q)

    def testValue(self):
        """
        test the value of the function by shuffling all particles
        """
        for rep in xrange(100):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos1 = random.uniform(-10,10)
                for j in xrange(10):
                    pos2 = random.uniform(-10,10)
                    observed = self.cov([pos1],[pos2])[0]
                    t = self.theta.get_nuisance()
                    l = self.lam.get_nuisance()
                    s = self.sig.get_nuisance()
                    if abs(pos1-pos2)<1e-7:
                        expected = s**2
                    else:
                        expected=s**2*t*exp(-0.5*(abs(pos1-pos2)/l)**2)
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected ,delta=0.001)
                pos2=pos1
                observed = self.cov([pos1],[pos2])[0]
                t = self.theta.get_nuisance()
                l = self.lam.get_nuisance()
                s = self.sig.get_nuisance()
                expected=s**2
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)

    def testDerivOne(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on one point at a time
        """
        skipped = 0
        for rep in xrange(10):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos1 = random.uniform(-10,10)
                for j in xrange(10):
                    pos2 = random.uniform(-10,10)
                    t = self.theta.get_nuisance()
                    l = self.lam.get_nuisance()
                    s = self.sig.get_nuisance()
                    self.cov.add_to_derivatives([pos1],[pos2],self.DA)
                    #theta
                    if abs(pos1-pos2)<1e-7:
                        expected = 0
                    else:
                        expected=s**2*exp(-0.5*(abs(pos1-pos2)/l)**2)
                    observed = self.theta.get_nuisance_derivative()
                    if isnan(expected):
                        skipped += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
                    #lambda
                    if abs(pos1-pos2)<1e-7:
                        expected=0
                    else:
                        expected = s**2*t*exp(-0.5*(abs(pos1-pos2)/l)**2)*abs(pos1-pos2)**2/l**3
                    observed = self.lam.get_nuisance_derivative()
                    if isnan(expected):
                        skipped += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
                    #sigma
                    if abs(pos1-pos2)<1e-7:
                        expected = 2*s
                    else:
                        expected=2*s*t*exp(-0.5*(abs(pos1-pos2)/l)**2)
                    observed = self.sig.get_nuisance_derivative()
                    if isnan(expected):
                        skipped += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
                    self.theta.add_to_nuisance_derivative(-self.theta.get_nuisance_derivative(),self.DA)
                    self.lam.add_to_nuisance_derivative(-self.lam.get_nuisance_derivative(),self.DA)
                    self.sig.add_to_nuisance_derivative(-self.sig.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

    def testDerivOneSame(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on one point at a time when pos1==pos2
        """
        skipped = 0
        for rep in xrange(10):
            self.shuffle_particle_values()
            for i in xrange(10):
                pos1 = random.uniform(-10,10)
                pos2 = pos1
                t = self.theta.get_nuisance()
                l = self.lam.get_nuisance()
                s = self.sig.get_nuisance()
                self.cov.add_to_derivatives([pos1],[pos2],self.DA)
                #theta
                expected = 0
                observed = self.theta.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #lambda
                expected=0
                observed = self.lam.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #sigma
                expected = 2*s
                observed = self.sig.get_nuisance_derivative()
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                self.theta.add_to_nuisance_derivative(-self.theta.get_nuisance_derivative(),self.DA)
                self.lam.add_to_nuisance_derivative(-self.lam.get_nuisance_derivative(),self.DA)
                self.sig.add_to_nuisance_derivative(-self.sig.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

if __name__ == '__main__':
    IMP.test.main()


