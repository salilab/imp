#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,JeffreysRestraint,NOERestraint

#unit testing framework
import IMP.test

class TestNOERestraintSimple(IMP.test.TestCase):
    "simple test cases to check if NOERestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.gamma = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.p0=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(0,0,0))
        self.p1=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(1,1,1))
        self.DA = IMP.DerivativeAccumulator()
        self.Jsi = IMP.isd.JeffreysRestraint(self.sigma)
        self.Jga = IMP.isd.JeffreysRestraint(self.gamma)
        self.V_obs=3.0
        self.noe = IMP.isd.NOERestraint(self.p0,self.p1,
                self.sigma, self.gamma, self.V_obs)

    def testValuePDist(self):
        "Test NOERestraint probability by changing distance"
        # Probability should be equal to
        # 1/(sqrt(2*pi)*sigma*V_obs) * exp(-1/(2sigma^2)*log^2(gamma*d^-6/Vobs))
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=self.sigma.get_scale()
            gamma=self.gamma.get_scale()
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            p1.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            dist=IMP.core.get_distance(p0,p1)
            expected=1/(sqrt(2*pi)*no*self.V_obs)*exp(\
                    -1/(2*no**2)*log(gamma*dist**-6/self.V_obs)**2)
            self.assertAlmostEqual(self.noe.get_probability(),
                    expected,delta=0.001)

    def testValuePSigma(self):
        "Test NOERestraint probability by changing sigma"
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            gamma=self.gamma.get_scale()
            dist=IMP.core.get_distance(p0,p1)
            expected=1/(sqrt(2*pi)*no*self.V_obs)*exp(\
                    -1/(2*no**2)*log(gamma*dist**-6/self.V_obs)**2)
            self.assertAlmostEqual(self.noe.get_probability(),
                    expected,delta=0.001)

    def testValuePGamma(self):
        "Test NOERestraint probability by changing gamma"
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=self.sigma.get_scale()
            gamma=uniform(0.1,100)
            self.gamma.set_scale(gamma)
            dist=IMP.core.get_distance(p0,p1)
            expected=1/(sqrt(2*pi)*no*self.V_obs)*exp(\
                    -1/(2*no**2)*log(gamma*dist**-6/self.V_obs)**2)
            self.assertAlmostEqual(self.noe.get_probability(),
                    expected,delta=0.001)

    def testValueChi(self):
        "Test NOERestraint value of chi by changing distance"
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=self.sigma.get_scale()
            gamma=self.gamma.get_scale()
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            p1.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            dist=IMP.core.get_distance(p0,p1)
            expected=-log(gamma*dist**-6/self.V_obs)
            self.noe.unprotected_evaluate(None)
            self.assertAlmostEqual(self.noe.get_chi(),
                    expected,delta=0.001)

    def testValueEDist(self):
        "Test NOERestraint score by changing distance"
        # Score should be equal to
        # 0.5*log(2*pi) + log(sigma*V_obs) + 1/(2sigma^2)*log^2(gamma*d^-6/Vobs)
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=self.sigma.get_scale()
            gamma=self.gamma.get_scale()
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            p1.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            dist=IMP.core.get_distance(p0,p1)
            expected=0.5*log(2*pi) + log(no*self.V_obs) + \
                    1/(2*no**2)*log(gamma*dist**-6/self.V_obs)**2
            self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                    expected,delta=0.001)

    def testValueESigma(self):
        "Test NOERestraint score by changing sigma"
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            gamma=self.gamma.get_scale()
            dist=IMP.core.get_distance(p0,p1)
            expected=0.5*log(2*pi) + log(no*self.V_obs) + \
                    1/(2*no**2)*log(gamma*dist**-6/self.V_obs)**2
            self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                    expected,delta=0.001)

    def testValueEGamma(self):
        "Test NOERestraint score by changing gamma"
        for i in xrange(100):
            p0=self.p0
            p1=self.p1
            no=self.sigma.get_scale()
            gamma=uniform(0.1,100)
            self.gamma.set_scale(gamma)
            dist=IMP.core.get_distance(p0,p1)
            expected=0.5*log(2*pi) + log(no*self.V_obs) + \
                    1/(2*no**2)*log(gamma*dist**-6/self.V_obs)**2
            self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                    expected,delta=0.001)

    def testParticles(self):
        "Test NOERestraint::get_input_particles"
        self.assertEqual(self.noe.get_input_particles(),
                [self.p0,self.p1,self.sigma,self.gamma])

    def testContainers(self):
        "Test NOERestraint::get_input_containers"
        self.assertEqual(self.noe.get_input_containers(),[])

    def testDerivativeX(self):
        "Test NOERestraint derivative w/r to X"
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            pos0=[uniform(0.1,100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1=[uniform(0.1,100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            dist=sqrt(sum([(i-j)**2 for (i,j) in zip(pos0,pos1)]))
            sigma=uniform(0.1,100)
            self.sigma.set_scale(sigma)
            gamma=uniform(0.1,100)
            self.gamma.set_scale(gamma)
            self.m.evaluate(self.DA)
            for coord in range(3):
                self.assertAlmostEqual(self.p0.get_derivative(coord),
                    (pos0[coord]-pos1[coord])/dist*(-6/dist)
                    *(-1/sigma**2*log(self.V_obs/(gamma*dist**-6))),
                    delta=0.001)
                self.assertAlmostEqual(self.p1.get_derivative(coord),
                    (pos1[coord]-pos0[coord])/dist*(-6/dist)
                    *(-1/sigma**2*log(self.V_obs/(gamma*dist**-6))),
                    delta=0.001)

    def testDerivativeSigma(self):
        "Test NOERestraint derivative w/r to sigma"
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            pos0=[uniform(0.1,100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1=[uniform(0.1,100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            dist=sqrt(sum([(i-j)**2 for (i,j) in zip(pos0,pos1)]))
            sigma=uniform(0.1,100)
            self.sigma.set_scale(sigma)
            gamma=uniform(0.1,100)
            self.gamma.set_scale(gamma)
            self.m.evaluate(self.DA)
            self.assertAlmostEqual(self.sigma.get_scale_derivative(),
                    1/sigma-1/sigma**3*log(self.V_obs/(gamma*dist**-6))**2,
                    delta=0.001)

    def testDerivativeGamma(self):
        "Test NOERestraint derivative w/r to gamma"
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            pos0=[uniform(0.1,100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1=[uniform(0.1,100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            dist=sqrt(sum([(i-j)**2 for (i,j) in zip(pos0,pos1)]))
            sigma=uniform(0.1,100)
            self.sigma.set_scale(sigma)
            gamma=uniform(0.1,100)
            self.gamma.set_scale(gamma)
            self.m.evaluate(self.DA)
            self.assertAlmostEqual(self.gamma.get_scale_derivative(),
                    1/gamma*(-1/sigma**2*log(self.V_obs/(gamma*dist**-6))),
                    delta=0.001)

    def testSanityEP(self):
        "Test if NOE score is -log(prob)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.noe.unprotected_evaluate(self.DA),
                    -log(self.noe.get_probability()),delta=0.001)

    def testSanityPE(self):
        "Test if NOE prob is exp(-score)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.noe.get_probability(),
                    exp(-self.noe.unprotected_evaluate(self.DA)),delta=0.001)

class TestNOERestraintApplied(IMP.test.TestCase):
    "tests the NOE restraint in simulations"

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.gamma = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.p0=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(0,0,0))
        self.p1=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(1,1,1))
        self.DA = IMP.DerivativeAccumulator()
        self.Jsi = IMP.isd.JeffreysRestraint(self.sigma)
        self.Jga = IMP.isd.JeffreysRestraint(self.gamma)
        self.V_obs=3.0
        self.noe = IMP.isd.NOERestraint(self.p0,self.p1,
                self.sigma, self.gamma, self.V_obs)

    def testSimpleOptimization(self):
        "Tests to satisfy an NOERestraint between two points"
        m=self.m
        self.gamma.set_scale(3.0)
        m.add_restraint(self.noe) #this leads to a target distance of 1.0
        self.p1.set_coordinates_are_optimized(True)
        # set the conjugate gradient minimization
        opt=IMP.core.ConjugateGradients(m)
        # perform the minimization with 100 steps
        opt.optimize(100)
        self.assertAlmostEqual(IMP.core.get_distance(self.p0,self.p1),
                1.0,delta=0.001)

if __name__ == '__main__':
    IMP.test.main()
