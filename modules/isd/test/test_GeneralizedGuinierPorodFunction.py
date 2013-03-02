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
    """ test of Generalized Guinier Porod function """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.G = Scale.setup_particle(IMP.Particle(self.m), 10.0)
        self.Rg = Scale.setup_particle(IMP.Particle(self.m), 10.0)
        self.d = Scale.setup_particle(IMP.Particle(self.m), 4.0)
        self.s = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.A = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.particles = [self.G, self.Rg, self.d, self.s, self.A]
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
        A = self.A.get_nuisance()
        Q1 = ((d-s)*(3-s)/2)**.5/Rg
        D = G*exp(-(d-s)/2)*Q1**(d-s)
        return G,Rg,d,s,A,Q1,D

    def get_value(self, q):
        G,Rg,d,s,A,Q1,D = self.get_params()
        if q <= Q1:
            return A+(G/(q**s))*exp(-(q*Rg)**2/(3.-s))
        else:
            return A+D/(q**d)

    def get_deriv_G(self, q):
        A = self.A.get_nuisance()
        return (self.get_value(q)-A)/self.G.get_nuisance();

    def get_deriv_Rg(self, q):
        G,Rg,d,s,A,Q1,D = self.get_params()
        if q <= Q1:
            return (self.get_value(q)-A)*(- (2*q**2 * Rg)/(3-s))
        else:
            return (self.get_value(q)-A)*(s-d)/Rg

    def get_deriv_d(self, q):
        G,Rg,d,s,A,Q1,D = self.get_params()
        if q <= Q1:
            return 0
        else:
            return (self.get_value(q)-A)*log(Q1/q)

    def get_deriv_s(self, q):
        G,Rg,d,s,A,Q1,D = self.get_params()
        if q <= Q1:
            return -(self.get_value(q)-A)*( (q*Rg)**2/(s-3)**2 + log(q))
        else:
            return (self.get_value(q)-A)*((d-s)/(2*(s-3)) - log(Q1))

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
                #A
                observed = self.A.get_nuisance_derivative()
                expected = 1
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
                self.A.add_to_nuisance_derivative(
                        -self.A.get_nuisance_derivative(),self.DA)
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

    def testDerivNumericG(self):
        """
        test the derivatives of the function numerically for G
        """
        particle=0
        self.G.set_nuisance(5)
        pos = self.get_params()[5]
        GFunc = MockFunc(self.G.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for G in xrange(1,10):
            self.G.set_nuisance(G)
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],
                    False)[0][particle]
            expected = IMP.test.numerical_derivative(GFunc, G, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericRg(self):
        """
        test the derivatives of the function numerically for Rg
        """
        particle=1
        self.Rg.set_nuisance(5)
        pos = self.get_params()[5]
        RgFunc = MockFunc(self.Rg.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for Rg in xrange(1,10):
            self.Rg.set_nuisance(Rg)
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],
                    False)[0][particle]
            expected = IMP.test.numerical_derivative(RgFunc, Rg, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericd(self):
        """
        test the derivatives of the function numerically for d
        """
        particle=2
        self.d.set_nuisance(2)
        pos = self.get_params()[5]
        dFunc = MockFunc(self.d.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for d in linspace(4,0.1):
            self.d.set_nuisance(d)
            # Function only valid for d>s, so ensure s is always a little below
            # d so we can calculate numerical derivatives
            if self.s.get_nuisance() + 0.04 > d:
                self.s.set_nuisance(random.uniform(0.05,min(d-0.04,2.9)))
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],
                    False)[0][particle]
            expected = IMP.test.numerical_derivative(dFunc, d, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testDerivNumerics(self):
        """
        test the derivatives of the function numerically for s
        """
        particle=3
        self.s.set_nuisance(1.5)
        pos = self.get_params()[5]
        sFunc = MockFunc(self.s.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for s in linspace(0.1,2.9,num=20): # can't compute derivative at border
            self.s.set_nuisance(s)
            if self.d.get_nuisance() < s:
                self.d.set_nuisance(random.uniform(s,4))
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],
                    False)[0][particle]
            expected = IMP.test.numerical_derivative(sFunc, s, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testDerivNumericA(self):
        """
        test the derivatives of the function numerically for A
        """
        particle=4
        self.A.set_nuisance(5)
        pos = self.get_params()[4]
        AFunc = MockFunc(self.A.set_nuisance,
                lambda a: self.mean([a])[0], pos, update=self.mean.update)
        for A in xrange(1,10):
            self.A.set_nuisance(A)
            self.mean.update()
            observed = self.mean.get_derivative_matrix([[pos]],
                    False)[0][particle]
            expected = IMP.test.numerical_derivative(AFunc, A, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericA(self):
        """
        test the Hessian of the function numerically wrt A and any other
        particle
        """
        pa=4
        for pb in xrange(4):
            self.G.set_nuisance(5)
            pos = self.get_params()[5]
            GFunc = MockFunc(self.particles[pb].set_nuisance,
                    lambda a: self.mean.get_derivative_matrix([[a]],
                            False)[0][pa], pos, update=self.mean.update)
            for part in xrange(1,10):
                self.particles[pb].set_nuisance(part)
                self.mean.update()
                observed = self.mean.get_second_derivative_vector(pa,
                        pb, [[pos]], False)[0][0]
                expected = IMP.test.numerical_derivative(GFunc, part, 0.01)
                self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericGG(self):
        """
        test the Hessian of the function numerically wrt G and G
        """
        pa=0
        pb=0
        self.G.set_nuisance(5)
        pos = self.get_params()[5]
        GFunc = MockFunc(self.G.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for G in xrange(1,10):
            self.G.set_nuisance(G)
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(GFunc, G, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericGRg(self):
        """
        test the Hessian of the function numerically wrt G and Rg
        """
        pa=0
        pb=1
        self.Rg.set_nuisance(5)
        pos = self.get_params()[5]
        RgFunc = MockFunc(self.Rg.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for Rg in xrange(1,10):
            self.Rg.set_nuisance(Rg)
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(RgFunc, Rg, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericRgG(self):
        """
        test the Hessian of the function numerically wrt Rg and G
        """
        pa=1
        pb=0
        self.Rg.set_nuisance(5)
        pos = self.get_params()[5]
        GFunc = MockFunc(self.G.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for G in xrange(1,10):
            self.G.set_nuisance(G)
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(GFunc, G, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericGd(self):
        """
        test the Hessian of the function numerically wrt G and d
        """
        pa=0
        pb=2
        self.d.set_nuisance(2)
        pos = self.get_params()[5]
        dFunc = MockFunc(self.d.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for d in linspace(4,0.1):
            self.d.set_nuisance(d)
            if self.s.get_nuisance() > d - 0.02:
                self.s.set_nuisance(random.uniform(0.02,min(d,3)-0.02))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(dFunc, d, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericGs(self):
        """
        test the Hessian of the function numerically wrt G and s
        """
        pa=0
        pb=3
        self.s.set_nuisance(1.5)
        pos = self.get_params()[5]
        sFunc = MockFunc(self.s.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for s in linspace(0.1,2.9,num=20):
            self.s.set_nuisance(s)
            if self.d.get_nuisance() < s:
                self.d.set_nuisance(random.uniform(s,4))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(sFunc, s, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericRgRg(self):
        """
        test the Hessian of the function numerically wrt Rg and Rg
        """
        pa=1
        pb=1
        self.Rg.set_nuisance(5)
        pos = self.get_params()[5]
        RgFunc = MockFunc(self.Rg.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for Rg in linspace(0.1,9.1):
            self.Rg.set_nuisance(Rg)
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(RgFunc, Rg, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericRgd(self):
        """
        test the Hessian of the function numerically wrt Rg and d
        """
        pa=1
        pb=2
        self.d.set_nuisance(2)
        pos = self.get_params()[5]
        dFunc = MockFunc(self.d.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for d in linspace(4,0.1,num=20):
            self.d.set_nuisance(d)
            if self.s.get_nuisance() > d - 0.02:
                self.s.set_nuisance(random.uniform(0.07,min(d,3)-0.02))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(dFunc, d, 0.005)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericRgs(self):
        """
        test the Hessian of the function numerically wrt Rg and s
        """
        pa=1
        pb=3
        self.s.set_nuisance(1.5)
        pos = self.get_params()[5]
        sFunc = MockFunc(self.s.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for s in linspace(0.1,2.9,num=20):
            self.s.set_nuisance(s)
            if self.d.get_nuisance() < s:
                self.d.set_nuisance(random.uniform(s,4))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(sFunc, s, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testHessianNumericdd(self):
        """
        test the Hessian of the function numerically wrt d and d
        """
        pa=2
        pb=2
        self.d.set_nuisance(2)
        pos = self.get_params()[5]
        dFunc = MockFunc(self.d.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for d in linspace(4,0.1,num=20):
            self.d.set_nuisance(d)
            if self.s.get_nuisance() > d - 0.02:
                self.s.set_nuisance(random.uniform(0.07,min(d,3)-0.02))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(dFunc, d, 0.005)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericds(self):
        """
        test the Hessian of the function numerically wrt d and s
        """
        pa=2
        pb=3
        self.s.set_nuisance(1.5)
        pos = self.get_params()[5]
        sFunc = MockFunc(self.s.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for s in linspace(0.1,2.9,num=20):
            self.s.set_nuisance(s)
            if self.d.get_nuisance() < s:
                self.d.set_nuisance(random.uniform(s,4))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(sFunc, s, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testHessianNumericss(self):
        """
        test the Hessian of the function numerically wrt s and s
        """
        pa=3
        pb=3
        self.s.set_nuisance(1.5)
        pos = self.get_params()[5]
        sFunc = MockFunc(self.s.set_nuisance,
                lambda a: self.mean.get_derivative_matrix([[a]], False)[0][pa],
                        pos, update=self.mean.update)
        for s in linspace(0.1,2.9,num=20):
            self.s.set_nuisance(s)
            if self.d.get_nuisance() < s:
                self.d.set_nuisance(random.uniform(s+0.1,4))
            self.mean.update()
            observed = self.mean.get_second_derivative_vector(pa, pb, [[pos]],
                    False)[0][0]
            expected = IMP.test.numerical_derivative(sFunc, s, 0.01)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testGetDerivativeMatrix(self):
        for rep in xrange(3):
            xlist = random.uniform(0,1, random.randint(1,100))
            data = self.mean.get_derivative_matrix([[i] for  i in xlist], True)
            self.assertEqual(len(data), len(xlist))
            self.assertEqual(len(data[0]), 5)
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
