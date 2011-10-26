#!/usr/bin/env python

#general imports
from numpy import *


#imp general
import IMP

#our project
from IMP.isd import *

#unit testing framework
import IMP.test

class TestCovariance1DFunction(IMP.test.TestCase):
    """ test of tau**2 exp( -(q-q')**2/(2 lambda**2) )  """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()
        IMP.set_log_level(0)
        self.tau = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.lam = Scale.setup_particle(IMP.Particle(self.m),  1.0)
        self.particles=[self.tau,self.lam]
        self.cov = Covariance1DFunction(*self.particles)
        self.DA=IMP.DerivativeAccumulator()

    def shuffle_particle_values(self):
        particles = [(self.tau,    0, 10),
                     (self.lam,    0, 10)]
        #number of shuffled values
        for i in xrange(random.randint(0,len(particles))):
            #which particle
            p,imin,imax = particles.pop(random.randint(0,len(particles)))
            p.set_scale(random.uniform(imin, imax))
        self.cov.update()

    def test_has_changed(self):
        for p in self.tau, self.lam:
            p.set_nuisance(p.get_nuisance()+1)
            self.assertTrue(self.cov.has_changed())
            self.cov.update()
            self.assertFalse(self.cov.has_changed())

    def testFail(self):
        q=IMP.Particle(self.m)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, self.tau,q)
        self.assertRaises(IMP.InternalException, Covariance1DFunction, q,self.tau)

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
                    expected=t**2*exp(-0.5*(abs(pos1-pos2)/l)**2)
                    if abs(expected) > 1e-7:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected ,delta=0.001)
                pos2=pos1
                observed = self.cov([pos1],[pos2])[0]
                t = self.tau.get_nuisance()
                l = self.lam.get_nuisance()
                expected=t**2*exp(-0.5*(abs(pos1-pos2)/l)**2)
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
                    self.tau.add_to_nuisance_derivative(-self.tau.get_nuisance_derivative(),self.DA)
                    self.lam.add_to_nuisance_derivative(-self.lam.get_nuisance_derivative(),self.DA)
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
                self.tau.add_to_nuisance_derivative(-self.tau.get_nuisance_derivative(),self.DA)
                self.lam.add_to_nuisance_derivative(-self.lam.get_nuisance_derivative(),self.DA)
        if skipped > 10:
            self.fail("too many NANs")

    def testValues(self):
        """
        tests if we can get multiple values at once
        """
        for rep in xrange(10):
            self.shuffle_particle_values()
            data = random.uniform(-10,10,random.randint(100))
            expected = [self.cov([i],[j]) for i in data for j in data]
            observed = self.cov([[i] for i in data],True)
            self.assertEqual(observed,expected)

    def testGetDerivativeMatrix(self):
        for rep in xrange(3):
            self.shuffle_particle_values()
            #xlist = random.uniform(-10,10,random.randint(100))
            xlist = random.uniform(-10,10,3)
            for ipart,part in enumerate(self.particles):
                data = self.cov.get_derivative_matrix(ipart,
                        [[i] for  i in xlist], True)
                self.assertEqual(len(data), len(xlist))
                self.assertEqual(len(data[0]), len(xlist))
                for (i,j),obs in ndenumerate(data):
                    self.cov.add_to_derivatives([xlist[i]],[xlist[j]], self.DA)
                    expected=part.get_nuisance_derivative()
                    if abs(expected)>1e-7:
                        self.assertAlmostEqual(obs/expected,1, delta=1e-5)
                    else:
                        self.assertAlmostEqual(obs, 0, delta=1e-5)
                    for p in self.particles:
                        p.add_to_nuisance_derivative(
                                -p.get_nuisance_derivative(), self.DA)

    def testAddToParticleDerivative(self):
        for i in xrange(10):
            for ipart,part in enumerate(self.particles):
                val = random.uniform(-10,10)
                self.cov.add_to_particle_derivative(ipart, val, self.DA)
                for jpart in xrange(2):
                    if ipart == jpart:
                        self.assertAlmostEqual(
                                part.get_nuisance_derivative(), val)
                        part.add_to_nuisance_derivative(-val, self.DA)
                    else:
                        self.assertAlmostEqual(
                            self.particles[jpart].get_nuisance_derivative(),
                            0.0)

if __name__ == '__main__':
    IMP.test.main()
