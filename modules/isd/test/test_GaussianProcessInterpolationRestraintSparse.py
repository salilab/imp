#!/usr/bin/env python

#general imports
from numpy import *
from random import *


#imp general
import IMP

#our project
import IMP.isd
from IMP.isd import *

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    """test of the GPI restraint with two data points, linear prior mean and
    gaussian prior covariances
    """

    def setUp(self):
        if not IMP.isd.IMP_ISD_HAS_CHOLMOD:
            self.skipTest("IMP not built with CHOLMOD")
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.q=[[0],[1]]
        self.I=[1,1]
        self.err=array([sqrt(10),sqrt(20)])
        self.N=[10,10]
        self.alpha = Nuisance.setup_particle(IMP.Particle(self.m), 0.0)
        self.beta = Nuisance.setup_particle(IMP.Particle(self.m),  0.0)
        self.mean = Linear1DFunction(self.alpha,self.beta)
        self.tau = Switching.setup_particle(IMP.Particle(self.m), 1.0)
        self.lam = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.sig = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.cov = Covariance1DFunction(self.tau, self.lam, self.sig, 2.0)
        self.gpi = IMP.isd.GaussianProcessInterpolationSparse(self.q, self.I,
                self.err, self.N, self.mean, self.cov)
        self.gpr = IMP.isd.GaussianProcessInterpolationRestraintSparse(self.gpi)
        self.m.add_restraint(self.gpr)

    def get_probability(self):
        N=self.N[0]
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        W=array([[self.cov(i,j)[0] for j in self.q] for i in self.q])
        obserr=diag(self.err*self.err/N)
        P=linalg.inv(W)
        eps=transpose([I-m])
        ret = exp(-N/2.*dot(dot(transpose(eps),P),eps) \
             - 1/2.*trace(dot(obserr,P)))               \
             * ((2*pi)**M*linalg.det(W))**(-N/2.)
        assert ret.shape==(1,1)
        return ret[0,0]

    def get_energy(self):
        N=self.N[0]
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        W=array([[self.cov(i,j)[0] for j in self.q] for i in self.q])
        obserr=diag(self.err*self.err/N)
        P=linalg.inv(W)
        eps=transpose([I-m])
        ret =  N/2.*dot(dot(transpose(eps),P),eps)   \
             + 1/2.*trace(dot(obserr,P))             \
             + N/2.*log((2*pi)**M*linalg.det(W))
        assert ret.shape==(1,1)
        return ret[0,0]

    def get_dmvndmu(self):
        N=self.N[0]
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        W=array([[self.cov(i,j)[0] for j in self.q] for i in self.q])
        P=linalg.inv(W)
        eps=transpose([I-m])
        return -N*dot(P,eps)

    def get_dmvndcov(self):
        N=self.N[0]
        M=len(self.q)
        I=array(self.I)
        m=array([self.mean(i)[0] for i in self.q])
        W=array([[self.cov(i,j)[0] for j in self.q] for i in self.q])
        obserr=diag(self.err*self.err/N)
        P=linalg.inv(W)
        eps=transpose([I-m])
        se=dot(P,eps)
        return -N/2.*dot(se,transpose(se))           \
               -1/2.*dot(dot(transpose(P),obserr),P) \
               +N/2.*transpose(P)

    def get_dmudalpha(self,q):
        return q

    def get_dmudbeta(self,q):
        return 1

    def get_dcovdtau(self,q1,q2):
        t=self.tau.get_nuisance()
        l=self.lam.get_nuisance()
        return 2*t*exp(-(q2-q1)**2./(2*l**2))

    def get_dcovdsigma(self,q1,q2):
        if abs(q1-q2)<1e-7:
            return 2*self.sig.get_nuisance()
        else:
            return 0

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
        return ret[0,0]+ret[1,1]

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
        b=array([[self.get_dcovdsigma(q[0],r[0])
                for r in self.q]
                for q in self.q])
        ret=dot(transpose(a),b)
        assert ret.shape==(2,2)
        return ret[0,0]+ret[1,1]

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
        "Test the value of the GPI sparse restraint density by varying alpha"
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
        "Test the value of the GPI sparse restraint density by varying beta"
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
        "Test the value of the GPI sparse restraint density by varying tau"
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
        "Test the value of the GPI sparse restraint density by varying lambda"
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
        "Test the value of the GPI sparse restraint energy by varying alpha"
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
        "Test the value of the GPI sparse restraint energy by varying beta"
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
        "Test the value of the GPI sparse restraint energy by varying tau"
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
        "Test the value of the GPI sparse restraint energy by varying lambda"
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

    def testDerivativesAlpha(self):
        "Test derivatives of GPI sparse restraint by varying alpha"
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
        "Test derivatives of GPI sparse restraint by varying beta"
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
        "Test derivatives of GPI sparse restraint by varying tau"
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
        "Test derivatives of GPI sparse restraint by varying lambda"
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
        "Test the GPI sparse restraint energy by shuffling all particles"
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
