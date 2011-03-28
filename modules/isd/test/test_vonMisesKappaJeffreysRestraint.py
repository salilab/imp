#!/usr/bin/env python

#general imports
from numpy import *
from scipy.special import i0,i1
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,vonMisesKappaJeffreysRestraint

#unit testing framework
import IMP.test

class TestvonMisesKappaJeffreysRestraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.kappa = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.DA = IMP.DerivativeAccumulator()
        self.J = IMP.isd.vonMisesKappaJeffreysRestraint(self.kappa)
        self.m.add_restraint(self.J)

    def testValueP(self):
        "test probability"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    sqrt(ratio*(no-ratio-no*ratio*ratio)),
                    delta=0.001)

    def testValueE(self): 
        "test if score is log(scale)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(None),
                    -0.5*log(ratio*(no-ratio-no*ratio*ratio)),
                    delta=0.001)

    def testDerivative(self):
        "test the derivative of the restraint"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.m.evaluate(self.DA)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.kappa.get_scale_derivative(),
                    0.5*(-1/ratio+3*ratio+1/no+1/(no-no**2/ratio+ratio*no**2)),
                    delta=0.001)

    def testParticles(self):
        self.assertEqual(self.J.get_input_particles(),[self.kappa])

    def testContainers(self):
        self.assertEqual(self.J.get_input_containers(),[])

    def testNonzeroE(self):
        "raise ValueError if zero"
        self.kappa.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNegativeE(self):
        "raise ValueError if negative"
        self.kappa.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNonzeroP(self):
        "raise ValueError if zero"
        self.kappa.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)
        
    def testNegativeP(self):
        "raise ValueError if negative"
        self.kappa.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)

    def testSanityEP(self):
        "test if score is -log(prob)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                    -log(self.J.get_probability()))

    def testSanityPE(self):
        "test if prob is exp(-score)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    exp(-self.J.unprotected_evaluate(self.DA)))


if __name__ == '__main__':
    IMP.test.main()




        
        
        




        

