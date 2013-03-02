#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform

#imp general
import IMP

#our project
from IMP.isd import Scale,vonMisesKappaJeffreysRestraint

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.kappa = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.DA = IMP.DerivativeAccumulator()
        self.J = IMP.isd.vonMisesKappaJeffreysRestraint(self.kappa)
        self.m.add_restraint(self.J)

    def testValueP(self):
        "Test vonMisesKappaJeffreys probability"
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    sqrt(ratio*(no-ratio-no*ratio*ratio)),
                    delta=0.001)

    def testValueE(self):
        "Test if vonMisesKappaJeffreys score is log(scale)"
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(None),
                    -0.5*log(ratio*(no-ratio-no*ratio*ratio)),
                    delta=0.001)

    def testDerivative(self):
        "Test the derivative of vonMisesKappaJeffreysRestraint"
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.m.evaluate(self.DA)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.kappa.get_scale_derivative(),
                    0.5*(-1/ratio+3*ratio+1/no+1/(no-no**2/ratio+ratio*no**2)),
                    delta=0.001)

    def testParticles(self):
        "Test vonMisesKappaJeffreysRestraint::get_input_particles()"
        self.assertEqual(self.J.get_input_particles(),[self.kappa])

    def testContainers(self):
        "Test vonMisesKappaJeffreysRestraint::get_input_containers()"
        self.assertEqual(self.J.get_input_containers(),[])

    def testNonzeroE(self):
        "vonMisesKappaKappaJeffreys errors on evaluate with zero scale"
        self.kappa.set_scale(0.0)
        self.assertRaises(IMP.base.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNegativeE(self):
        "vonMisesKappaKappaJeffreys errors on evaluate with negative scale"
        self.kappa.set_scale(-1.0)
        self.assertRaises(IMP.base.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNonzeroP(self):
        "Test vonMisesKappaKappaJeffreys get_prob with zero scale"
        self.kappa.set_scale(0.0)
        self.assertRaises(IMP.base.ModelException, self.J.get_probability)

    def testNegativeP(self):
        "Test vonMisesKappaKappaJeffreys get_prob with negative scale"
        self.kappa.set_scale(-1.0)
        self.assertRaises(IMP.base.ModelException, self.J.get_probability)

    def testSanityEP(self):
        "Test if vonMisesKappaJeffreys score is -log(prob)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                    -log(self.J.get_probability()))

    def testSanityPE(self):
        "Test if vonMisesKappaJeffreys prob is exp(-score)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    exp(-self.J.unprotected_evaluate(self.DA)))


if __name__ == '__main__':
    IMP.test.main()
