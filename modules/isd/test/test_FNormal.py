#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import FNormal

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)

    def testEvaluate(self):
        "tests FNormal.evaluate"
        for i in xrange(100):
            randno = [uniform(-100,100), uniform(0.1,100),
                    uniform(-100,100),uniform(0.1,100)]
            fn=FNormal(*randno)
            self.assertAlmostEqual(fn.evaluate(),
                    0.5*log(2*pi) + log(randno[3]/randno[1])
                    + 0.5/randno[3]**2*(randno[0]-randno[2])**2,
                    delta=0.001)

    def testEvaluateDFA(self):
        "tests FNormal.evaluate_derivative_FA"
        for i in xrange(100):
            randno = [uniform(-100,100), uniform(0.1,100),
                    uniform(-100,100),uniform(0.1,100)]
            fn=FNormal(*randno)
            self.assertAlmostEqual(fn.evaluate_derivative_FA(),
                    (randno[0]-randno[2])/randno[3]**2,
                    delta=0.001)

    def testEvaluateDJA(self):
        "tests FNormal.evaluate_derivative_JA"
        for i in xrange(100):
            randno = [uniform(-100,100), uniform(0.1,100),
                    uniform(-100,100),uniform(0.1,100)]
            fn=FNormal(*randno)
            self.assertAlmostEqual(fn.evaluate_derivative_JA(),
                    -1/randno[1],delta=0.001)

    def testEvaluateDFM(self):
        "tests FNormal.evaluate_derivative_FM"
        for i in xrange(100):
            randno = [uniform(-100,100), uniform(0.1,100),
                    uniform(-100,100),uniform(0.1,100)]
            fn=FNormal(*randno)
            self.assertAlmostEqual(fn.evaluate_derivative_FM(),
                    (randno[2]-randno[0])/randno[3]**2,
                    delta=0.001)

    def testEvaluateDSigma(self):
        "tests FNormal.evaluate_derivative_sigma"
        for i in xrange(100):
            randno = [uniform(-100,100), uniform(0.1,100),
                    uniform(-100,100),uniform(0.1,100)]
            fn=FNormal(*randno)
            self.assertAlmostEqual(fn.evaluate_derivative_sigma(),
                    1/randno[3]-(randno[0]-randno[2])**2/randno[3]**3,
                    delta=0.001)

    def testDensity(self):
        "tests FNormal.density"
        for i in xrange(100):
            randno = [uniform(-100,100), uniform(0.1,100),
                    uniform(-100,100),uniform(0.1,100)]
            fn=FNormal(*randno)
            self.assertAlmostEqual(fn.density(),
                    randno[1]/(sqrt(2*pi)*randno[3])
                    *exp(-(randno[0]-randno[2])**2/(2*randno[3]**2)),
                    delta=0.001)

if __name__ == '__main__':
    IMP.test.main()
