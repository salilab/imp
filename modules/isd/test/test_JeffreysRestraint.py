#!/usr/bin/env python

# general imports
from numpy import *
from random import uniform
import pickle

# imp general
import IMP

# our project
from IMP.isd import Scale, JeffreysRestraint

# unit testing framework
import IMP.test


class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.DA = IMP.DerivativeAccumulator()
        self.J = IMP.isd.JeffreysRestraint(self.m, self.sigma)

    def testValueP(self):
        "Test if JeffreysRestraint probability is 1/scale"
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                                   1.0 / no, delta=0.001)

    def testValueE(self):
        "Test if JeffreysRestraint score is log(scale)"
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                                   log(no), delta=0.001)

    def testDerivative(self):
        "Test the derivative of JeffreysRestraint"
        sf = IMP.core.RestraintsScoringFunction([self.J])
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            sf.evaluate(True)
            self.assertAlmostEqual(self.sigma.get_scale_derivative(),
                                   1.0 / no, delta=0.001)

    def test_get_inputs(self):
        "Test JeffreysRestraint::get_inputs()"
        self.assertEqual([x.get_name() for x in self.J.get_inputs()],
                         [self.sigma.get_name()])

    def testNonzeroE(self):
        "JeffreysRestraint raise ValueError on evaluate with zero scale"
        self.sigma.set_scale(0.0)
        self.assertRaises(
            IMP.ModelException,
            self.J.unprotected_evaluate,
            self.DA)

    def testNegativeE(self):
        "JeffreysRestraint raise ValueError on evaluate with negative scale"
        self.sigma.set_scale(-1.0)
        self.assertRaises(
            IMP.ModelException,
            self.J.unprotected_evaluate,
            self.DA)

    def testNonzeroP(self):
        "JeffreysRestraint raise ValueError on get_prob with zero scale"
        self.sigma.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)

    def testNegativeP(self):
        "JeffreysRestraint raise ValueError on get_prob with negative scale"
        self.sigma.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)

    def testSanityEP(self):
        "Test if JeffreysRestraint score is -log(prob)"
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                                   -log(self.J.get_probability()))

    def testSanityPE(self):
        "Test if JeffreysRestraint prob is exp(-score)"
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                                   exp(-self.J.unprotected_evaluate(self.DA)))

    def test_serialize(self):
        """Test (un-)serialize of JeffreysRestraint"""
        self.sigma.set_scale(50.)
        self.J.set_name("foo")
        self.assertAlmostEqual(self.J.evaluate(True), 3.912, delta=0.001)
        dump = pickle.dumps(self.J)
        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(True), 3.912, delta=0.001)

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of JeffreysRestraint via polymorphic pointer"""
        self.sigma.set_scale(50.)
        self.J.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([self.J])
        self.assertAlmostEqual(sf.evaluate(True), 3.912, delta=0.001)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(True), 3.912, delta=0.001)


if __name__ == '__main__':
    IMP.test.main()
