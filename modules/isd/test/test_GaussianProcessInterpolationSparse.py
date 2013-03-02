#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
import IMP.isd
from IMP.isd import *

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    """test of the GPI with two data points, linear prior mean and gaussian
    prior covariances
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
        self.err=[sqrt(10),sqrt(20)]
        self.N=[10,10]
        self.alpha = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.beta = Nuisance.setup_particle(IMP.Particle(self.m),  1.0)
        self.mean = Linear1DFunction(self.alpha,self.beta)
        self.tau = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.lam = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.sig = Scale.setup_particle(IMP.Particle(self.m), 0.0)
        self.cov = Covariance1DFunction(self.tau, self.lam, self.sig)
        self.gpi = IMP.isd.GaussianProcessInterpolationSparse(self.q, self.I,
                self.err, self.N, self.mean, self.cov)

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

    def testValuePosteriorMeanAlpha(self):
        """
        test the value of the posterior mean function between 0 and 1 by
        changing alpha
        """
        for a in linspace(-10,10,num=10):
            self.alpha.set_nuisance(a)
            for q in linspace(0,1,num=10):
                observed = self.gpi.get_posterior_mean([q])
                expected = self.get_I(q)
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)

    def testValuePosteriorMeanBeta(self):
        """
        test the value of the posterior mean function between 0 and 1 by
        changing beta
        """
        for b in linspace(-10,10,num=10):
            self.beta.set_nuisance(b)
            for q in linspace(0,1,num=10):
                observed = self.gpi.get_posterior_mean([q])
                expected = self.get_I(q)
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)

    def testValuePosteriorMeanTau(self):
        """
        test the value of the posterior mean function between 0 and 1 by
        changing tau
        """
        for t in linspace(0.01,10,num=10):
            self.tau.set_nuisance(t)
            for q in linspace(0,1,num=10):
                observed = self.gpi.get_posterior_mean([q])
                expected = self.get_I(q)
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)

    def testValuePosteriorMeanTau2(self):
        """
        test the value of the posterior mean function between 0 and 1 by
        changing tau and getting the covariance first.
        """
        for t in linspace(0.01,10,num=10):
            self.tau.set_nuisance(t)
            for q in linspace(0,1,num=10):
                #calling this updates the W matrix
                self.gpi.get_posterior_covariance([q],[q])
                #this only works if WSIm_ is updated
                observed = self.gpi.get_posterior_mean([q])
                expected = self.get_I(q)
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)

    def testValuePosteriorMeanLambda(self):
        """
        test the value of the posterior mean function between 0 and 1 by
        changing lambda
        """
        skipnan =0
        for l in linspace(0.01,10,num=10):
            self.lam.set_nuisance(l)
            #fl=open('out%d'%(int(l)),'w')
            for q in linspace(0,1,num=10):
                #print "posterior mean lambda=",l,"q=",q
                observed = self.gpi.get_posterior_mean([q])
                expected = self.get_I(q)
                if isnan(expected):
                    skipnan += 1
                    continue
                #print "PYTHON:",q,l,observed,expected
                #fl.write(' '.join(['%G' % i for i in [q,l,observed,expected]]))
                #fl.write('\n')
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
        if skipnan > 10: # less than 10%
            self.fail("too much NANs")

    def testValuePosteriorMeanSigma(self):
        """
        test the value of the posterior mean function between 0 and 1 by
        changing sigma
        """
        skipnan =0
        for s in linspace(0.01,10,num=10):
            self.sig.set_nuisance(s)
            #fl=open('out%d'%(int(s)),'w')
            for q in linspace(0,1,num=10):
                #print "posterior mean sigma=",s,"q=",q
                observed = self.gpi.get_posterior_mean([q])
                expected = self.get_I(q)
                if isnan(expected):
                    skipnan += 1
                    continue
                #print "PYTHON:",q,s,observed,expected
                #fl.write(' '.join(['%G' % i for i in [q,s,observed,expected]]))
                #fl.write('\n')
                if expected != 0:
                    self.assertAlmostEqual(observed/expected
                        ,1.0,delta=0.001)
                else:
                    self.assertAlmostEqual(observed,expected
                        ,delta=0.001)
        if skipnan > 10: # less than 10%
            self.fail("too much NANs")
    #("need to update test mean function")
    testValuePosteriorMeanSigma = \
              IMP.test.expectedFailure(testValuePosteriorMeanSigma)

    def testValuePosteriorCovarianceTau(self):
        """
        test the value of the posterior covariance function between 0 and 1 by
        changing tau
        """
        for t in linspace(0.01,10,num=10):
            self.tau.set_nuisance(t)
            #fl=open('out%d'%(int(t)),'w')
            for q1 in linspace(0,1,num=10):
                for q2 in linspace(0,1,num=10):
                    #print "posterior mean tau=",t,"q1=",q1,"q2=",q2
                    observed = self.gpi.get_posterior_covariance([q1],[q2])
                    expected = self.get_cov(q1,q2)
                    #print "PYTHON:",q1,q2,t,observed,expected
                    #fl.write(' '.join(['%G' % i
                    #    for i in [q1,q2,t,observed,expected]]))
                    #fl.write('\n')
                    #continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)

    def testValuePosteriorCovarianceLambda(self):
        """
        test the value of the posterior covariance function between 0 and 1 by
        changing lambda
        """
        skipnan = 0
        for l in linspace(0.01,10,num=10):
            self.lam.set_nuisance(l)
            #fl=open('out%d'%(int(l)),'w')
            for q1 in linspace(0,1,num=10):
                for q2 in linspace(0,1,num=10):
                    #print "posterior mean lambda=",l,"q1=",q1,"q2=",q2
                    observed = self.gpi.get_posterior_covariance([q1],[q2])
                    expected = self.get_cov(q1,q2)
                    #print "PYTHON:",q1,q2,t,observed,expected
                    #fl.write(' '.join(['%G' % i
                    #    for i in [q1,q2,l,observed,expected]]))
                    #fl.write('\n')
                    #continue
                    if isnan(expected):
                        skipnan += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
        if skipnan > 100: # less than 10%
            self.fail("too much NANs")

    def testValuePosteriorCovarianceSigma(self):
        """
        test the value of the posterior covariance function between 0 and 1 by
        changing sigma
        """
        skipnan = 0
        for s in linspace(0.01,10,num=10):
            self.sig.set_nuisance(s)
            #fl=open('out%d'%(int(s)),'w')
            for q1 in linspace(0,1,num=10):
                for q2 in linspace(0,1,num=10):
                    #print "posterior mean sigma=",s,"q1=",q1,"q2=",q2
                    observed = self.gpi.get_posterior_covariance([q1],[q2])
                    expected = self.get_cov(q1,q2)
                    #print "PYTHON:",q1,q2,t,observed,expected
                    #fl.write(' '.join(['%G' % i
                    #    for i in [q1,q2,s,observed,expected]]))
                    #fl.write('\n')
                    #continue
                    if isnan(expected):
                        skipnan += 1
                        continue
                    if expected != 0:
                        self.assertAlmostEqual(observed/expected
                            ,1.0,delta=0.001)
                    else:
                        self.assertAlmostEqual(observed,expected
                            ,delta=0.001)
        if skipnan > 100: # less than 10%
            self.fail("too much NANs")
    #("need to update test covariance function")
    testValuePosteriorCovarianceSigma = \
           IMP.test.expectedFailure(testValuePosteriorCovarianceSigma)

if __name__ == '__main__':
    IMP.test.main()
