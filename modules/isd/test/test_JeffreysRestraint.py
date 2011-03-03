#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,JeffreysRestraint

#unit testing framework
import IMP.test

class TestJeffreysRestraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.DA = IMP.DerivativeAccumulator()
        self.J = IMP.isd.JeffreysRestraint(self.sigma)

    def testValueP(self):
        "test if probability is 1/scale"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    1.0/no,delta=0.001)

    def testValueE(self): 
        "test if score is log(scale)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                    log(no), delta=0.001)

    def testDerivative(self):
        "test the derivative of the restraint"
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.m.evaluate(self.DA)
            self.assertAlmostEqual(self.sigma.get_scale_derivative(),
                    1.0/no,delta=0.001)

    def testParticles(self):
        self.assertEqual(self.J.get_input_particles(),[self.sigma])

    def testContainers(self):
        self.assertEqual(self.J.get_input_containers(),[])

    def testNonzeroE(self):
        "raise ValueError if zero"
        self.sigma.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNegativeE(self):
        "raise ValueError if negative"
        self.sigma.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNonzeroP(self):
        "raise ValueError if zero"
        self.sigma.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)
        
    def testNegativeP(self):
        "raise ValueError if negative"
        self.sigma.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)

    def testSanityEP(self):
        "test if score is -log(prob)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                    -log(self.J.get_probability()))

    def testSanityPE(self):
        "test if prob is exp(-score)"
        for i in xrange(100):
            no=uniform(0.1,100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    exp(-self.J.unprotected_evaluate(self.DA)))


if __name__ == '__main__':
    IMP.test.main()




        
        
        




        

