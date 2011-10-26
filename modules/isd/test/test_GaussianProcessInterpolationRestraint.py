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
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.q=[[0],[1]]
        self.I=[1,1]
        self.err=array([sqrt(10),sqrt(20)])
        self.N=10
        self.alpha = Nuisance.setup_particle(IMP.Particle(self.m), 0.0)
        self.alpha.set_nuisance_is_optimized(True)
        self.beta = Nuisance.setup_particle(IMP.Particle(self.m),  0.0)
        self.beta.set_nuisance_is_optimized(True)
        self.mean = Linear1DFunction(self.alpha,self.beta)
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

    def get_Omega(self):
        N=self.N
        W=array([[self.cov(i,j)[0] for j in self.q] for i in self.q])
        S=diag(self.err*self.err)
        sigma=self.sig.get_nuisance()
        Omega=W+sigma*S/N
        return Omega

    def get_probability(self):
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        Omega=self.get_Omega()
        P=linalg.inv(Omega)
        eps=transpose([I-m])
        ret = exp(-1/2.*dot(dot(transpose(eps),P),eps)) \
             * ((2*pi)**M*linalg.det(Omega))**(-1/2.)
        assert ret.shape==(1,1)
        return ret[0,0]

    def get_energy(self):
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        Omega=self.get_Omega()
        P=linalg.inv(Omega)
        eps=transpose([I-m])
        ret =  1/2.*dot(dot(transpose(eps),P),eps)   \
             + 1/2.*log((2*pi)**M*linalg.det(Omega))
        assert ret.shape==(1,1)
        return ret[0,0]

    def get_dmvndmu(self):
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        Omega=self.get_Omega()
        P=linalg.inv(Omega)
        eps=transpose([I-m])
        return -dot(P,eps)

    def get_dmvndcov(self):
        N=self.N
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        Omega=self.get_Omega()
        P=linalg.inv(Omega)
        eps=transpose([I-m])
        se=dot(P,eps)
        return -1/2.*dot(se,transpose(se))           \
               +1/2.*transpose(P)

    def get_dmudalpha(self,q):
        return q

    def get_dmudbeta(self,q):
        return 1

    def get_dcovdtau(self,q1,q2):
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return 2*t*exp(-(q2-q1)**2./(2*l**2))

    def get_dcovdlambda(self,q1,q2):
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return t**2*exp(-0.5*(abs(q2-q1)/l)**2)*abs(q2-q1)**2/l**3

    def get_derivative_alpha(self):
        a=self.get_dmvndmu()
        b=array([self.get_dmudalpha(q[0]) for q in self.q])
        ret=dot(transpose(a),b)
        assert ret.shape==(1,)
        return ret[0]

    def get_derivative_beta(self):
        a=self.get_dmvndmu()
        b=array([self.get_dmudbeta(q[0]) for q in self.q])
        ret=dot(transpose(a),b)
        assert ret.shape==(1,)
        return ret[0]

    def get_derivative_lambda(self):
        a=self.get_dmvndcov()
        b=array([[self.get_dcovdlambda(q[0],r[0])
                for r in self.q]
                for q in self.q])
        ret=dot(transpose(a),b)
        assert ret.shape==(2,2)
        return ret[0,0]+ret[1,1]  #return the trace

    def get_derivative_tau(self):
        a=self.get_dmvndcov()
        b=array([[self.get_dcovdtau(q[0],r[0])
                for r in self.q]
                for q in self.q])
        ret=dot(transpose(a),b)
        assert ret.shape==(2,2)
        return ret[0,0]+ret[1,1]

    def get_derivative_sigma(self):
        a=self.get_dmvndcov()
        S=diag(self.err*self.err)
        N=self.N
        ret=dot(transpose(a),S/N)
        assert ret.shape==(2,2)
        return ret[0,0]+ret[1,1]

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

    def testValueDensitySigma(self):
        """
        test the value of the density by varying sigma
        """
        skipped = 0
        for a in logspace(-1,2,num=100):
            self.sig.set_nuisance(a)
            self.m.evaluate(False)
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
        test the value of the energy by varying alpha
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
        test the value of the energy by varying beta
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
        test the value of the energy by varying tau
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
        test the value of the energy by varying lambda
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

    def testValueEnergySigma(self):
        """
        test the value of the energy by varying sigma
        """
        skipped = 0
        for a in logspace(-1,2,num=100):
            self.sig.set_nuisance(a)
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
        test the value of the energy by shuffling all particles
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
        for a in linspace(-10,10,num=11):
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
            expected=observed
            expected = self.get_derivative_tau()
            continue
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #lambda
            observed = self.lam.get_nuisance_derivative()
            expected=observed
            expected = self.get_derivative_lambda()
            if expected != 0:
                self.assertAlmostEqual(observed/expected
                    ,1.0,delta=0.001)
            else:
                self.assertAlmostEqual(observed,expected
                    ,delta=0.001)
            #sigma
            observed = self.sig.get_nuisance_derivative()
            expected = self.get_derivative_sigma()
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

    def testDerivativesSigma(self):
        """test derivatives by varying sigma"""
        skipped = 0
        for a in logspace(-0,2,num=100):
            self.sig.set_nuisance(a)
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

    def testDerivativesAll(self):
        """
        test the derivatives by shuffling all particles
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

#class TestGaussianProcessInterpolationRestraintCG2Points(
#        TestGaussianProcessInterpolationRestraint2Points):
#    def setUp(self):
#        TestGaussianProcessInterpolationRestraint2Points.setUp(self)
#        self.gpr.set_use_cg(True,1e-7)

if __name__ == '__main__':
    IMP.test.main()
