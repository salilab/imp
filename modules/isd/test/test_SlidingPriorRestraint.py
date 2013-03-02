#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,SlidingPriorRestraint

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.q0 = Scale.setup_particle(IMP.Particle(self.m), 0.1)
        self.DA = IMP.DerivativeAccumulator()
        self.qmin = 0.01
        self.qmax = 0.8
        self.sq = 0.34
        self.J = IMP.isd.SlidingPriorRestraint(self.q0, self.qmin, self.qmax,
                self.sq)

    def testValueP(self):
        "Test SlidingPriorRestraint probability"
        for i in xrange(100):
            no=uniform(self.qmin, self.qmax)
            self.q0.set_scale(no)
            sq = self.sq*(self.qmax-self.qmin)
            expected = exp(0.5*((no - self.qmin)/sq)**2)/sq
            self.assertAlmostEqual(self.J.get_probability(),
                    expected,delta=0.001)

    def testValueE(self):
        "Test SlidingPriorRestraint score"
        for i in xrange(100):
            no=uniform(self.qmin, self.qmax)
            self.q0.set_scale(no)
            sq = self.sq*(self.qmax-self.qmin)
            expected = - 0.5*((no - self.qmin)/sq)**2 + log(sq)
            self.assertAlmostEqual(self.J.unprotected_evaluate(None),
                    expected,delta=0.001)

    def testDerivative(self):
        "Test the derivative of SlidingPriorRestraint"
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(self.qmin, self.qmax)
            self.q0.set_scale(no)
            self.m.evaluate(self.DA)
            sq = self.sq*(self.qmax-self.qmin)
            expected = -(no-self.qmin)/sq**2
            self.assertAlmostEqual(self.q0.get_scale_derivative(),
                    expected,delta=0.001)

    def testParticles(self):
        "Test SlidingPriorRestraint::get_input_particles()"
        self.assertEqual(self.J.get_input_particles(),[self.q0])

    def testContainers(self):
        "Test SlidingPriorRestraint::get_input_containers()"
        self.assertEqual(self.J.get_input_containers(),[])

    def testNonzeroE(self):
        "Raise ValueError on eval if SlidingPriorRestraint scale out of bounds"
        self.q0.set_scale(self.qmin-1)
        self.assertRaises(IMP.base.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNonzeroP(self):
        "Raise ValueError on prob if SlidingPriorRestraint scale out of bounds"
        self.q0.set_scale(self.qmin-1)
        self.assertRaises(IMP.base.ModelException, self.J.get_probability)

    def testSanityEP(self):
        "Test SlidingPrior if score is -log(prob)"
        for i in xrange(100):
            no=uniform(self.qmin, self.qmax)
            self.q0.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                    -log(self.J.get_probability()))

    def testSanityPE(self):
        "Test if SlidingPrior prob is exp(-score)"
        for i in xrange(100):
            no=uniform(self.qmin, self.qmax)
            self.q0.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    exp(-self.J.unprotected_evaluate(self.DA)))


if __name__ == '__main__':
    IMP.test.main()
