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

class TestGaussianProcessInterpolationRestraint2Points(IMP.test.TestCase):
    """test of the GPI restraint with two data points, linear prior mean and
    gaussian prior covariances
    """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.q=[[0],[1]]
        self.I=[1,1]
        self.err=[sqrt(10),sqrt(20)]
        self.N=[10,10]
        self.alpha = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.beta = Nuisance.setup_particle(IMP.Particle(self.m),  1.0)
        self.mean = Linear1DFunction(self.alpha,self.beta)
        self.tau = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.lam = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.cov = Covariance1DFunction(2.0, self.tau, self.lam)
        self.gpi = IMP.isd.GaussianProcessInterpolation(self.q, self.I,
                self.err, self.N, self.mean, self.cov)
        self.gpr = IMP.isd.GaussianProcessInterpolationRestraint(self.gpi)
        self.m.add_restraint(self.gpr)

    def get_I(self, q):
        a=self.alpha.get_nuisance()
        b=self.beta.get_nuisance()
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return q*a+b+1./(exp(1./l**2)*(2+3*t**2+t**4)-t**4) \
                *(exp(-q**2/(2*l**2))*(b-1)*t**2*(exp(q/l**2)*t**2
                                            -exp(1./l**2)*(2+t**2))
                 -exp(-(q**2-1)/(2*l**2))*(b+a-1)*t**2*(-t**2 +
                                            exp(q/l**2)*(1+t**2)))

    def get_cov(self, q1, q2):
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return t**2*exp(-(q1-q2)**2/(2.*l**2)) \
           -  t**4*exp(-(q1**2+q2**2)/(2.*l**2))\
                    /(-t**4+exp(1./l**2)*(2.+3.*t**2+t**4)) \
               *(exp((q1+q2)/l**2)*(1.+t**2) - t**2*(exp(q1/l**2)+exp(q2/l**2))
                                + exp(1./l**2)*(2.+t**2))

    def get_probability(self):
        """2x2 multivariate normal density"""
        q1=self.q[0][0]
        q2=self.q[1][0]
        a=self.get_I(q1)
        b=self.get_I(q2)
        c=self.get_cov(q1,q1)
        d=self.get_cov(q1,q2)
        e=self.get_cov(q2,q1)
        f=self.get_cov(q2,q2)
        det=c*f-d*e
        return 1./(2*pi*sqrt(det))*exp(0.5*(
            -(1-b)/det * (c*(1-b)-(d*(1-a)))
            -(1-a)/det * (-e*(1-b)+(f*(1-a)))))

    def get_energy(self):
        """2x2 multivariate normal -log(density)"""
        q1=self.q[0][0]
        q2=self.q[1][0]
        a=self.get_I(q1)
        b=self.get_I(q2)
        c=self.get_cov(q1,q1)
        d=self.get_cov(q1,q2)
        e=self.get_cov(q2,q1)
        f=self.get_cov(q2,q2)
        det=c*f-d*e
        return log(2*pi*sqrt(det)) \
                + 0.5*( (1-b)/det * (c*(1-b)-(d*(1-a))) +
            (1-a)/det * (-e*(1-b)+(f*(1-a))))

    def get_derivative_alpha(self):
        """derivative of MVN wrt alpha"""
        q1=self.q[0][0]
        q2=self.q[1][0]
        a=self.alpha.get_nuisance()
        b=self.beta.get_nuisance()
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return exp(1/l**2)*(\
                (-1+a+b)*t**2
                +2*exp(1/l**2)*(-1+a+b)*(1+t**2)
                -exp(1/(2*l**2))*(-1+b)*(2+3*t**2))\
               /((-1+exp(1/l**2))*t**2*(-t**4+exp(1/l**2)*(2+3*t**2+t**4)))

    def get_derivative_beta(self):
        """derivative of MVN wrt beta"""
        q1=self.q[0][0]
        q2=self.q[1][0]
        a=self.alpha.get_nuisance()
        b=self.beta.get_nuisance()
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return exp(1/l**2)*(
                -(-3+a+3*b)*t**2
                +exp(1/(2*l**2))*(2*a*(1+t**2)+(-1+b)*(4+3*t**2))
            )/((1+exp(1/(2*l**2)))*t**2*(-t**4+exp(1/l**2)*(2+3*t**2+t**4)))

    def get_derivative_tau(self):
        """derivative of MVN wrt tau"""
        q1=self.q[0][0]
        q2=self.q[1][0]
        a=self.alpha.get_nuisance()
        b=self.beta.get_nuisance()
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return -exp(1/l**2)/((exp(1/l**2)-1)*t**3*(t**4-exp(1/l**2)*(t**4
            +3*t**2+2))**2) * (
                exp(1/l**2)*t**2 * (
                    -3*t**2*(a**2+2*a*(b-1)+(b-2)*b-5)
                    +t**4*(17-2*a*(a+2*b-2))
                    +6*t**6 + 8)
                + exp(2/l**2) * (
                    4*a**2*(t**2+1)**3+8*a*(b-1)*(t**2+1)**3
                    +(6*(b-2)*b-7)*t**6
                    +3*(7*(b-2)*b+1)*t**4
                    +8*(3*(b-2)*b+2)*t**2
                    +8*(b-1)**2-3*t**8)
                -2*t**6*( a**2+2*a*(b-1) + 3*(b-2)*b+5)
                +12*(b-1)*exp(1/(2*l**2))*(t**2+1)*t**4*(a+b-1)
                -2*(b-1)*exp(3/(2*l**2))*(
                    6*t**6+15*t**4+12*t**2+4) * (a+b-1) -3*t**8 )

    def get_derivative_lambda(self):
        """derivative of MVN wrt lambda"""
        q1=self.q[0][0]
        q2=self.q[1][0]
        a=self.alpha.get_nuisance()
        b=self.beta.get_nuisance()
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return exp(1/l**2)*(
                3*exp(1/(2*l**2))*(-1+b)*(-1+a+b)*t**4*(2+3*t**2)
                -exp(5/(2*l**2))*(-1+b)*(-1+a+b)*(1+t**2)*(2+t**2)*(2+3*t**2)
                -t**6*(5+a**2+2*a*(-1+b)+3*(-2+b)*b+3*t**2)
                -exp(3/(2*l**2))*(-1+b)*(-1+a+b)*(4+12*t**2+13*t**4+6*t**6)
                +exp(1/l**2)*t**2*(4-4*(-1+a**2+2*a*(-1+b)+2*(-2+b)*b)*t**2
                    +(7-4*a**2-8*a*(-1+b)-6*(-2+b)*b)*t**4+6*t**6)
                +exp(2/l**2)*(
                    8*(-1+b)**2
                    +4*(5+6*(-2+b)*b)*t**2
                    +2*(7+13*(-2+b)*b)*t**4
                    +(-2+9*(-2+b)*b)*t**6
                    -3*t**8
                    +a**2*(1+t**2)*(4+8*t**2+5*t**4)
                    +2*a*(-1+b)*(1+t**2)*(4+8*t**2+5*t**4)
                    ))/(
                (-1+exp(1/l**2))**2*l**3*t**2*
                    (t**4-exp(1/l**2)*(2+3*t**2+t**4))**2)

    def shuffle_particle_values(self):
        particles = [(self.alpha, -10, 10),
                     (self.beta, -10, 10),
                     (self.tau, 0.001, 10),
                     (self.lam, 0.1, 10)]
        #number of shuffled values
        for i in xrange(randint(0,4)):
            #which particle
            p,imin,imax = particles.pop(randint(0,len(particles)-1))
            p.set_nuisance(uniform(imin, imax))

    def testValueDensityAlpha(self):
        """
        test the value of the density by varying alpha
        """
        for a in linspace(-10,10,num=100):
            self.alpha.set_nuisance(a)
            observed = self.gpr.get_probability()
            expected = self.get_probability()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)

    def testValueDensityBeta(self):
        """
        test the value of the density by varying beta
        """
        for a in linspace(-10,10,num=100):
            self.beta.set_nuisance(a)
            observed = self.gpr.get_probability()
            expected = self.get_probability()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)

    def testValueDensityTau(self):
        """
        test the value of the density by varying tau
        """
        for a in logspace(-3,2,num=100):
            self.tau.set_nuisance(a)
            observed = self.gpr.get_probability()
            expected = self.get_probability()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)

    def testValueDensityLambda(self):
        """
        test the value of the density by varying lambda
        """
        skipped = 0
        for a in logspace(-1,2,num=100):
            self.lam.set_nuisance(a)
            observed = self.gpr.get_probability()
            expected = self.get_probability()
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

    def testValueEnergyAlpha(self):
        """
        test the value of the density by varying alpha
        """
        skipped = 0
        for a in linspace(-10,10,num=100):
            self.alpha.set_nuisance(a)
            observed = self.m.evaluate(False)
            expected = self.get_energy()
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

    def testValueEnergyBeta(self):
        """
        test the value of the density by varying beta
        """
        skipped = 0
        for a in linspace(-10,10,num=100):
            self.beta.set_nuisance(a)
            observed = self.m.evaluate(False)
            expected = self.get_energy()
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

    def testValueEnergyTau(self):
        """
        test the value of the density by varying tau
        """
        skipped = 0
        for a in logspace(-1,2,num=100):
            self.tau.set_nuisance(a)
            observed = self.m.evaluate(False)
            expected = self.get_energy()
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

    def testValueEnergyLambda(self):
        """
        test the value of the density by varying lambda
        """
        skipped = 0
        for a in logspace(-1,2,num=100):
            self.lam.set_nuisance(a)
            observed = self.m.evaluate(False)
            expected = self.get_energy()
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

    def testValueEnergyAll(self):
        """
        test the value of the density by shuffling all particles
        """
        skipped = 0
        for rep in xrange(100):
            self.shuffle_particle_values()
            observed = self.m.evaluate(False)
            expected = self.get_energy()
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

    def testDerivativesAlpha(self):
        """test derivatives by varying alpha"""
        for a in linspace(-10,10,num=100):
            self.alpha.set_nuisance(a)
            self.m.evaluate(True)
            #alpha
            observed = self.alpha.get_nuisance_derivative()
            expected = self.get_derivative_alpha()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #beta
            observed = self.beta.get_nuisance_derivative()
            expected = self.get_derivative_beta()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #tau
            observed = self.tau.get_nuisance_derivative()
            expected = self.get_derivative_tau()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #lambda
            observed = self.lam.get_nuisance_derivative()
            expected = self.get_derivative_lambda()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)

    def testDerivativesBeta(self):
        """test derivatives by varying beta"""
        for a in linspace(-10,10,num=100):
            self.beta.set_nuisance(a)
            self.m.evaluate(True)
            #alpha
            observed = self.alpha.get_nuisance_derivative()
            expected = self.get_derivative_alpha()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #beta
            observed = self.beta.get_nuisance_derivative()
            expected = self.get_derivative_beta()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #tau
            observed = self.tau.get_nuisance_derivative()
            expected = self.get_derivative_tau()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #lambda
            observed = self.lam.get_nuisance_derivative()
            expected = self.get_derivative_lambda()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)

    def testDerivativesTau(self):
        """test derivatives by varying tau"""
        for a in logspace(-3,2,num=100):
            self.tau.set_nuisance(a)
            self.m.evaluate(True)
            #alpha
            observed = self.alpha.get_nuisance_derivative()
            expected = self.get_derivative_alpha()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #beta
            observed = self.beta.get_nuisance_derivative()
            expected = self.get_derivative_beta()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #tau
            observed = self.tau.get_nuisance_derivative()
            expected = self.get_derivative_tau()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #lambda
            observed = self.lam.get_nuisance_derivative()
            expected = self.get_derivative_lambda()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)

    def testDerivativesLambda(self):
        """test derivatives by varying lambda"""
        skipped = 0
        for a in logspace(-0,2,num=100):
            self.lam.set_nuisance(a)
            self.m.evaluate(True)
            #alpha
            observed = self.alpha.get_nuisance_derivative()
            expected = self.get_derivative_alpha()
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
            observed = self.beta.get_nuisance_derivative()
            expected = self.get_derivative_beta()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #tau
            observed = self.tau.get_nuisance_derivative()
            expected = self.get_derivative_tau()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #lambda
            observed = self.lam.get_nuisance_derivative()
            expected = self.get_derivative_lambda()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
        if skipped > 10:
            self.fail("too much NANs")

    def testValueEnergyAll(self):
        """
        test the value of the density by shuffling all particles
        """
        skipped = 0
        for rep in xrange(100):
            self.shuffle_particle_values()
            observed = self.m.evaluate(True)
            expected = self.get_energy()
            if isnan(expected):
                skipped += 1
                continue
            #alpha
            observed = self.alpha.get_nuisance_derivative()
            expected = self.get_derivative_alpha()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #beta
            observed = self.beta.get_nuisance_derivative()
            expected = self.get_derivative_beta()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #tau
            observed = self.tau.get_nuisance_derivative()
            expected = self.get_derivative_tau()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #lambda
            observed = self.lam.get_nuisance_derivative()
            expected = self.get_derivative_lambda()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
        if skipped > 10:
            self.fail("too many NANs")

if __name__ == '__main__':
    IMP.test.main()


