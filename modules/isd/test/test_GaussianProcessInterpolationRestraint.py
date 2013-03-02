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
        self.q=[[0],[1]]
        self.I=[1,1]
        self.err=array([sqrt(10),sqrt(20)])
        self.N=10
        self.alpha = Nuisance.setup_particle(IMP.Particle(self.m,"alpha"), 0.0)
        self.alpha.set_nuisance_is_optimized(True)
        self.beta = Nuisance.setup_particle(IMP.Particle(self.m,'beta'),  0.0)
        self.beta.set_nuisance_is_optimized(True)
        self.mean = Linear1DFunction(self.alpha,self.beta)
        self.tau = Switching.setup_particle(IMP.Particle(self.m,'tau'), 1.0)
        self.tau.set_nuisance_is_optimized(True)
        self.lam = Scale.setup_particle(IMP.Particle(self.m,'lambda'), 1.0)
        self.lam.set_nuisance_is_optimized(True)
        self.sig = Scale.setup_particle(IMP.Particle(self.m,'sigma'), 1.0)
        self.sig.set_nuisance_is_optimized(True)
        self.cov = Covariance1DFunction(self.tau, self.lam, 2.0)
        self.gpi = IMP.isd.GaussianProcessInterpolation(self.q, self.I,
                self.err, self.N, self.mean, self.cov, self.sig)
        self.gpr = IMP.isd.GaussianProcessInterpolationRestraint(self.gpi)
        self.m.add_restraint(self.gpr)
        self.particles=[self.alpha,self.beta,self.sig,self.tau,self.lam]

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


    def testEnergyTerms(self):
        "Test GPI restraint energy terms"
        for i in xrange(10):
            self.shuffle_particle_values()
            expected = self.m.evaluate(False)
            U = self.gpr.get_minus_exponent()
            V = self.gpr.get_minus_log_normalization()
            if expected != 0:
                self.assertAlmostEqual((U+V)/expected, 1., delta=1e-7)
            else:
                self.assertAlmostEqual(U+V,0.,delta=1e-7)

    def testGetInputThings(self):
        "Test GPI restraint get_input_*() methods"
        particles = list(set(IMP.get_input_particles([self.gpr])))
        self.assertTrue(self.lam in particles)
        particles.remove(self.lam)
        self.assertTrue(self.tau in particles)
        particles.remove(self.tau)
        self.assertTrue(self.sig in particles)
        particles.remove(self.sig)
        self.assertTrue(self.alpha in particles)
        particles.remove(self.alpha)
        self.assertTrue(self.beta in particles)
        particles.remove(self.beta)
        self.assertEqual(particles,[])
        self.assertEqual(IMP.get_input_containers([self.gpr]), [])

    def testValueDensityAlpha(self):
        "Test the value of the GPI restraint density by varying alpha"
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
        "Test the value of the GPI restraint density by varying beta"
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
        "Test the value of the GPI restraint density by varying tau"
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
        "Test the value of the GPI restraint density by varying lambda"
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
        "Test the value of the GPI restraint density by varying sigma"
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
        "Test the value of the GPI restraint energy by varying alpha"
        skipped = 0
        for a in linspace(-10,10,num=20):
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
        "Test the value of the GPI restraint energy by varying beta"
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
        "Test the value of the GPI restraint energy by varying tau"
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
        "Test the value of the GPI restraint energy by varying lambda"
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
        "Test the value of the GPI restraint energy by varying sigma"
        #dg=IMP.get_pruned_dependency_graph(self.m)
        #dg.show_graphviz(open('pgraph.dot','w'))
        #dg=IMP.get_dependency_graph(self.m)
        #dgi=IMP.get_vertex_index(dg)
        #mi=self.m.get_inputs()
        #dg.show_graphviz(open('graph.dot','w'))
        #print IMP.get_dependent_score_states(self.m,mi,dg,dgi)
        #return
        skipped = 0
        #dg=IMP.get_pruned_dependency_graph(self.m)
        #dg.show_graphviz(open('pgraph.dot','w'))
        #dg=IMP.get_dependency_graph(self.m)
        #dgi=IMP.get_vertex_index(dg)
        #mi=self.m.get_outputs()
        #dg.show_graphviz(open('graph.dot','w'))
        #print IMP.get_dependent_score_states(self.m,mi,dg,dgi)
        #return
        #IMP.base.set_log_level(IMP.base.TERSE)
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
        "Test the value of the GPI restraint energy by shuffling all particles"
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
        """Test GPI restraint derivatives by varying alpha"""
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
        """Test GPI restraint derivatives by varying beta"""
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
        """Test GPI restraint derivatives by varying tau"""
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
        """Test GPI restraint derivatives by varying lambda"""
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
        """Test GPI restraint derivatives by varying sigma"""
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
        "Test GPI restraint derivatives by shuffling all particles"
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
                    ,1.0,delta=0.0015)
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

    def testDerivNumericAlpha(self):
        "Test the derivatives of the GPI restraint numerically for alpha"
        pnum=0
        values=range(1,10)
        particle=self.particles[pnum]
        #PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        PFunc = MockFunc(particle.set_nuisance, lambda a:self.get_energy(),
                False, update=self.mean.update)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, 1.)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericBeta(self):
        "Test the derivatives of the GPI restraint numerically for beta"
        pnum=1
        values=range(1,10)
        particle=self.particles[pnum]
        #PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        PFunc = MockFunc(particle.set_nuisance, lambda a:self.get_energy(),
                False, update=self.mean.update)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, 1.)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

    def testDerivNumericTau(self):
        "Test the derivatives of the GPI restraint numerically for tau"
        #IMP.base.set_log_level(IMP.base.TERSE)
        pnum=3
        values=linspace(.1,.9)
        particle=self.particles[pnum]
        PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        #PFunc = MockFunc(particle.set_nuisance, lambda a:self.get_energy(),
        #        False, update=self.cov.update)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, .01)
            #print val,observed,expected,ene
            #continue
            self.assertAlmostEqual(expected,observed,delta=5e-2)

    def testDerivNumericLambda(self):
        "Test the derivatives of the GPI restraint numerically for lambda"
        pnum=4
        values=linspace(.3,2)
        particle=self.particles[pnum]
        #PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        PFunc = MockFunc(particle.set_nuisance, lambda a:self.get_energy(),
                False, update=self.cov.update)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, .2)
            self.assertAlmostEqual(expected,observed,delta=1e-2)

    def testDerivNumericSigma(self):
        "Test the derivatives of the GPI restraint numerically for sigma"
        pnum=2
        values=range(1,10)
        particle=self.particles[pnum]
        #PFunc = MockFunc(particle.set_nuisance, self.m.evaluate, False)
        PFunc = MockFunc(particle.set_nuisance, lambda a:self.get_energy(),
                False, update=self.cov.update)
        for val in values:
            particle.set_nuisance(val)
            ene=self.m.evaluate(True)
            observed = particle.get_nuisance_derivative()
            expected = IMP.test.numerical_derivative(PFunc, val, 1.)
            self.assertAlmostEqual(expected,observed,delta=1e-3)

if __name__ == '__main__':
    IMP.test.main()
