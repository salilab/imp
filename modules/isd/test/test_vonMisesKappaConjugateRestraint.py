#!/usr/bin/env python

#general imports
from numpy import *
from scipy.special import i0,i1
from random import uniform

#imp general
import IMP

#our project
from IMP.isd import Scale,vonMisesKappaConjugateRestraint

#unit testing framework
import IMP.test

class TestvonMisesKappaConjugateRestraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.kappa = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.DA = IMP.DerivativeAccumulator()

    def testValuePKappa(self):
        "test probability by changing kappa"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(0.1,75)
            self.kappa.set_scale(no)
            ratio=i1(no)/i0(no)
            py=exp(no*R0)/i0(no)**c
            cpp=self.J.get_probability()
            self.assertAlmostEqual(cpp,py,delta=0.001)

    def testValuePc(self):
        "test probability by changing c"
        R0=1.0
        no=1.0
        self.kappa.set_scale(no)
        for i in xrange(100):
            c=uniform(2.0,75)
            self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
            self.m.add_restraint(self.J)
            ratio=i1(no)/i0(no)
            py=exp(no*R0)/i0(no)**c
            cpp=self.J.get_probability()
            self.assertAlmostEqual(cpp,py,delta=0.001)
            self.m.remove_restraint(self.J)

    def testValuePR0(self):
        "test probability by changing R0"
        c=10.0
        no=1.0
        self.kappa.set_scale(no)
        for i in xrange(100):
            R0=uniform(0.0,10.0)
            self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
            self.m.add_restraint(self.J)
            ratio=i1(no)/i0(no)
            py=exp(no*R0)/i0(no)**c
            cpp=self.J.get_probability()
            self.assertAlmostEqual(cpp,py,delta=0.001)
            self.m.remove_restraint(self.J)

    def testValueEKappa(self):
        "test energy by changing kappa"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            ratio=i1(no)/i0(no)
            py=-no*R0 + c*log(i0(no))
            cpp=self.J.evaluate(None)
            self.assertAlmostEqual(cpp,py,delta=0.001)

    def testValueEc(self):
        "test energy by changing c"
        R0=1
        no=1.0
        self.kappa.set_scale(no)
        for i in xrange(100):
            c=uniform(1.0,100)
            self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
            self.m.add_restraint(self.J)
            ratio=i1(no)/i0(no)
            py=-no*R0 + c*log(i0(no))
            cpp=self.J.evaluate(None)
            self.assertAlmostEqual(cpp,py,delta=0.001)
            self.m.remove_restraint(self.J)

    def testValueER0(self):
        "test energy by changing R0"
        c=10.0
        no=1.0
        self.kappa.set_scale(no)
        for i in xrange(100):
            R0=uniform(0.0,10.0)
            self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
            self.m.add_restraint(self.J)
            ratio=i1(no)/i0(no)
            py=-no*R0 + c*log(i0(no))
            cpp=self.J.evaluate(None)
            self.assertAlmostEqual(cpp,py,delta=0.001)
            self.m.remove_restraint(self.J)

    def testDerivativeKappa(self):
        "test the derivative of the restraint by changing kappa"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.m.evaluate(self.DA)
            ratio=i1(no)/i0(no)
            self.assertAlmostEqual(self.kappa.get_scale_derivative(),
                    -R0 + c*i1(no)/i0(no), delta=0.001)

    def testParticles(self):
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        self.assertEqual(self.J.get_input_particles(),[self.kappa])

    def testContainers(self):
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        self.assertEqual(self.J.get_input_containers(),[])

    def testNonzeroE(self):
        "raise ValueError if zero"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        self.kappa.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.unprotected_evaluate, self.DA)

    def testNegativeE(self):
        "raise ModelException if negative"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        self.kappa.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.unprotected_evaluate, self.DA)

    def testR0(self):
        "raise ModelException if R0 < 0"
        c=10
        R0=-1
        self.assertRaises(IMP.ModelException,
                IMP.isd.vonMisesKappaConjugateRestraint, self.kappa,c,R0)

    def testR0c(self):
        "raise ModelException if R0 > c"
        c=10
        R0=20
        self.assertRaises(IMP.ModelException,
                IMP.isd.vonMisesKappaConjugateRestraint, self.kappa,c,R0)

    def testc(self):
        "raise ModelException if c < 0"
        c=-1
        R0=c
        self.assertRaises(IMP.ModelException,
                IMP.isd.vonMisesKappaConjugateRestraint, self.kappa,c,R0)

    def testNonzeroP(self):
        "raise ValueError if zero"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        self.kappa.set_scale(0.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)
        
    def testNegativeP(self):
        "raise ValueError if negative"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        self.kappa.set_scale(-1.0)
        self.assertRaises(IMP.ModelException, self.J.get_probability)

    def testSanityEP(self):
        "test if score is -log(prob)"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(0.1,75)  #can't go higher because overflow errors
            self.kappa.set_scale(no)
            prob=self.J.get_probability()
            self.assertAlmostEqual(self.J.unprotected_evaluate(self.DA),
                    -log(prob))

    def testSanityPE(self):
        "test if prob is exp(-score)"
        c=10
        R0=1
        self.J = IMP.isd.vonMisesKappaConjugateRestraint(self.kappa,c,R0)
        self.m.add_restraint(self.J)
        for i in xrange(100):
            no=uniform(0.1,100)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.J.get_probability(),
                    exp(-self.J.unprotected_evaluate(self.DA)))


if __name__ == '__main__':
    IMP.test.main()


