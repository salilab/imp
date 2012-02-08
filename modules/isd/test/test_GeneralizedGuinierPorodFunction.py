#!/usr/bin/env python

#general imports
from numpy import *


#imp general
import IMP

#our project
from IMP.isd import *

#unit testing framework
import IMP.test

class TestGeneralizedGuinierPorodFunction(IMP.test.TestCase):
    """ test of a*x + b function """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.G = Scale.setup_particle(IMP.Particle(self.m), 10.0)
        self.Rg = Scale.setup_particle(IMP.Particle(self.m), 10.0)
        self.d = Scale.setup_particle(IMP.Particle(self.m), 4.0)
        self.s = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.particles = [self.G, self.Rg, self.d, self.s]
        self.mean = GeneralizedGuinierPorodFunction(*self.particles)
        self.DA=IMP.DerivativeAccumulator()

    def shuffle_particle_values(self):
        particles = [(self.G, 0, 1000),
                     (self.Rg, 0, 100),
                     (self.d, 1, 4)]
        #number of shuffled values
        for i in xrange(random.randint(0,len(particles))):
            #which particle
            p,imin,imax = particles.pop(random.randint(0,len(particles)))
            p.set_nuisance(random.uniform(imin, imax))
        if random.randint(0,2) or self.s.get_nuisance()>self.d.get_nuisance():
            self.s.set_nuisance(random.uniform(0,min(3,self.d.get_nuisance())))
        self.mean.update()

    def test_has_changed(self):
        for p in self.particles:
            p.set_nuisance(p.get_nuisance()+1)
            self.assertTrue(self.mean.has_changed())
            self.mean.update()
            self.assertFalse(self.mean.has_changed())

    def get_params(self):
        G = self.G.get_nuisance()
        Rg = self.Rg.get_nuisance()
        d = self.d.get_nuisance()
        s = self.s.get_nuisance()
        Q1 = ((d-s)*(3-s)/2)**.5/Rg
        D = G*exp(-(d-s)/2)*Q1**(d-s)
        return G,Rg,d,s,Q1,D

    def get_value(self, q):
        G,Rg,d,s,Q1,D = self.get_params()
        if q <= Q1:
            return (G/(q**s))*exp(-(q*Rg)**2/(3.-s))
        else:
            return D/(q**d)

    def get_deriv_G(self, q):
        return self.get_value(q)/self.G.get_nuisance();

    def get_deriv_Rg(self, q):
        G,Rg,d,s,Q1,D = self.get_params()
        if q <= Q1:
            return self.get_value(q)*(- (2*q**2 * Rg)/(3-s))
        else:
            return self.get_value(q)*(s-d)/Rg

    def get_deriv_d(self, q):
        G,Rg,d,s,Q1,D = self.get_params()
        if q <= Q1:
            return 0
        else:
            return self.get_value(q)*log(Q1/q)

    def get_deriv_s(self, q):
        G,Rg,d,s,Q1,D = self.get_params()
        if q <= Q1:
            return -self.get_value(q)*( (q*Rg)**2/(s-3)**2 + log(q))
        else:
            return self.get_value(q)*((d-s)/(2*(s-3)) - log(Q1))

    def testValue(self):
        """
        test the value of the function by shuffling all particles
        """
        skipped = 0
        for rep in xrange(10):
            for i in xrange(10):
                pos = random.uniform(0,1)
                observed = self.mean([pos])[0]
                expected=self.get_value(pos)
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
            self.shuffle_particle_values()
        if skipped > 10:
            self.fail("too many NANs")

    def testValues(self):
        """
        tests if we can get multiple values at once
        """
        for rep in xrange(10):
            data = random.uniform(0,1,random.randint(1,100))
            expected = [self.mean([i]) for i in data]
            observed = self.mean([[i] for i in data],True)
            self.assertEqual(observed,expected)
            self.shuffle_particle_values()

    def testDerivOne(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on one point at a time
        """
        skipped = 0
        for rep in xrange(10):
            for i in xrange(10):
                pos = random.uniform(0,1)
                self.mean.add_to_derivatives([pos],self.DA)
                #G
                observed = self.G.get_nuisance_derivative()
                expected=self.get_deriv_G(pos)
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #Rg
                observed = self.Rg.get_nuisance_derivative()
                expected=self.get_deriv_Rg(pos)
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #d
                observed = self.d.get_nuisance_derivative()
                expected=self.get_deriv_d(pos)
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                #s
                observed = self.s.get_nuisance_derivative()
                expected=self.get_deriv_s(pos)
                if isnan(expected):
                    skipped += 1
                    continue
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
                self.G.add_to_nuisance_derivative(
                        -self.G.get_nuisance_derivative(),self.DA)
                self.Rg.add_to_nuisance_derivative(
                        -self.Rg.get_nuisance_derivative(),self.DA)
                self.d.add_to_nuisance_derivative(
                        -self.d.get_nuisance_derivative(),self.DA)
                self.s.add_to_nuisance_derivative(
                        -self.s.get_nuisance_derivative(),self.DA)
            self.shuffle_particle_values()
        if skipped > 10:
            self.fail("too many NANs")

    def testDerivMult(self):
        """
        test the derivatives of the function by shuffling all particles
        Test on multiple points at a time
        """
        for rep in xrange(10):
            expectedG=0
            expectedRg=0
            expectedd=0
            expecteds=0
            for i in xrange(10):
                pos = random.uniform(0,1)
                self.mean.add_to_derivatives([pos],self.DA)
                expectedG += self.get_deriv_G(pos)
                expectedRg += self.get_deriv_Rg(pos)
                expectedd += self.get_deriv_d(pos)
                expecteds += self.get_deriv_s(pos)
            #G
            observedG = self.G.get_nuisance_derivative()
            if expectedG != 0:
                self.assertAlmostEqual(observedG/expectedG
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observedG,expectedG
                    ,delta=0.001)
            #Rg
            observedRg = self.Rg.get_nuisance_derivative()
            if expectedRg != 0:
                self.assertAlmostEqual(observedRg/expectedRg
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observedRg,expectedRg
                    ,delta=0.001)
            #d
            observedd = self.d.get_nuisance_derivative()
            if expectedd != 0:
                self.assertAlmostEqual(observedd/expectedd
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observedd,expectedd
                    ,delta=0.001)
            #s
            observeds = self.s.get_nuisance_derivative()
            if expecteds != 0:
                self.assertAlmostEqual(observeds/expecteds
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observeds,expecteds
                    ,delta=0.001)
            self.G.add_to_nuisance_derivative(-self.G.get_nuisance_derivative(),self.DA)
            self.Rg.add_to_nuisance_derivative(-self.Rg.get_nuisance_derivative(),self.DA)
            self.d.add_to_nuisance_derivative(-self.d.get_nuisance_derivative(),self.DA)
            self.s.add_to_nuisance_derivative(-self.s.get_nuisance_derivative(),self.DA)
            self.shuffle_particle_values()

    def testGetDerivativeMatrix(self):
        for rep in xrange(3):
            xlist = random.uniform(0,1, random.randint(1,100))
            data = self.mean.get_derivative_matrix([[i] for  i in xlist], True)
            self.assertEqual(len(data), len(xlist))
            self.assertEqual(len(data[0]), 4)
            for i,j in zip(data,xlist):
                if i[0] != 0:
                    self.assertAlmostEqual(self.get_deriv_G(j)/i[0],1,
                            delta=1e-5)
                else:
                    self.assertAlmostEqual(self.get_deriv_G(j), 0., delta=1e-5)
                if i[1] != 0:
                    self.assertAlmostEqual(self.get_deriv_Rg(j)/i[1],1,
                            delta=1e-5)
                else:
                    self.assertAlmostEqual(self.get_deriv_Rg(j), 0., delta=1e-5)
                if i[2] != 0:
                    self.assertAlmostEqual(self.get_deriv_d(j)/i[2],1,
                            delta=1e-5)
                else:
                    self.assertAlmostEqual(self.get_deriv_d(j), 0., delta=1e-5)
                if i[3] != 0:
                    self.assertAlmostEqual(self.get_deriv_s(j)/i[3],1,
                            delta=1e-5)
                else:
                    self.assertAlmostEqual(self.get_deriv_s(j), 0., delta=1e-5)
            self.shuffle_particle_values()

    def testAddToParticleDerivative(self):
        for i in xrange(10):
            #G
            val = random.uniform(0,1)
            self.mean.add_to_particle_derivative(0, val, self.DA)
            self.assertAlmostEqual(self.G.get_nuisance_derivative(), val)
            self.assertAlmostEqual(self.Rg.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.d.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.s.get_nuisance_derivative(), 0.0)
            self.G.add_to_nuisance_derivative(
                    -self.G.get_nuisance_derivative(),self.DA)
            #Rg
            val = random.uniform(0,1)
            self.mean.add_to_particle_derivative(1, val, self.DA)
            self.assertAlmostEqual(self.G.get_nuisance_derivative(), 0.)
            self.assertAlmostEqual(self.Rg.get_nuisance_derivative(), val)
            self.assertAlmostEqual(self.d.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.s.get_nuisance_derivative(), 0.0)
            self.Rg.add_to_nuisance_derivative(
                    -self.Rg.get_nuisance_derivative(),self.DA)
            #d
            val = random.uniform(0,1)
            self.mean.add_to_particle_derivative(2, val, self.DA)
            self.assertAlmostEqual(self.G.get_nuisance_derivative(), 0.)
            self.assertAlmostEqual(self.Rg.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.d.get_nuisance_derivative(), val)
            self.assertAlmostEqual(self.s.get_nuisance_derivative(), 0.0)
            self.d.add_to_nuisance_derivative(
                    -self.d.get_nuisance_derivative(),self.DA)
            #s
            val = random.uniform(0,1)
            self.mean.add_to_particle_derivative(3, val, self.DA)
            self.assertAlmostEqual(self.G.get_nuisance_derivative(), 0)
            self.assertAlmostEqual(self.Rg.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.d.get_nuisance_derivative(), 0.0)
            self.assertAlmostEqual(self.s.get_nuisance_derivative(), val)
            self.s.add_to_nuisance_derivative(
                    -self.s.get_nuisance_derivative(),self.DA)

if __name__ == '__main__':
    IMP.test.main()
