#!/usr/bin/env python

#general imports
from numpy import *
from scipy.special import i0,i1
from random import uniform,randint


#imp general
import IMP

#our project
from IMP.isd import vonMisesSufficient, vonMises

#unit testing framework
import IMP.test

class TestvonMisesSufficient(IMP.test.TestCase):
    """Tests the von Mises distribution for N observations using the 
    sufficient statistics formulation
    """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)

    def testEvaluate(self):
        "tests vonMisesSufficient.evaluate"
        for i in xrange(100):
            #x, N, cmu1, smu2, kappa
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            cmu1=uniform(-1,1)
            smu2=uniform(-1,1)
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,cmu1,smu2,kappa)
            self.assertAlmostEqual(fn.evaluate(),
                    log(2*pi*i0(kappa)**N) - N*kappa*(cos(x)*cmu1+sin(x)*smu2),
                    delta=0.001)

    def testEvaluateDX(self):
        "tests vonMisesSufficient.evaluate_derivative_x"
        for i in xrange(100):
            #x, N, cmu1, smu2, kappa
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            cmu1=uniform(-1,1)
            smu2=uniform(-1,1)
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,cmu1,smu2,kappa)
            self.assertAlmostEqual(fn.evaluate_derivative_x(),
                    N*kappa*sin(x)*cmu1 - N*kappa*cos(x)*smu2,
                    delta=0.001)

    def testEvaluateDKappa(self):
        "tests vonMisesSufficient.evaluate_derivative_kappa"
        for i in xrange(100):
            #x, N, cmu1, smu2, kappa
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            cmu1=uniform(-1,1)
            smu2=uniform(-1,1)
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,cmu1,smu2,kappa)
            self.assertAlmostEqual(fn.evaluate_derivative_kappa(),
                    N*i1(kappa)/i0(kappa) - N*(cos(x)*cmu1 + sin(x)*smu2),
                    delta=0.001)

    def testDensity(self):
        "tests vonMisesSufficient.density"
        for i in xrange(100):
            #x, N, cmu1, smu2, kappa
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            cmu1=uniform(-1,1)
            smu2=uniform(-1,1)
            kappa=uniform(0.1,10)
            fn=vonMisesSufficient(x,N,cmu1,smu2,kappa)
            cpp=fn.density()
            py=exp(N*kappa*(cos(x)*cmu1+sin(x)*smu2))/(2*pi*i0(kappa)**N)
            if py == 0.0:
               self.assertAlmostEqual(cpp,0.0,delta=0.001)
            else:
               self.assertAlmostEqual(cpp/py,1.0,delta=0.001)

class TestvonMisesSufficientDegenerate(IMP.test.TestCase):
    """the sufficient von Mises should reduce to the von Mises 
    when N=1 and mu1=mu2
    """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)

    def testEvaluate(self):
        "tests vonMisesSufficient.evaluate"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,cos(randno[1]),sin(randno[1]),randno[2])
            self.assertAlmostEqual(fn.evaluate(), fn2.evaluate(), delta=0.001)

    def testEvaluateDX(self):
        "tests vonMisesSufficient.evaluate_derivative_x"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,cos(randno[1]),sin(randno[1]),randno[2])
            self.assertAlmostEqual(fn.evaluate_derivative_x(),
                    fn2.evaluate_derivative_x(),
                    delta=0.001)

    def testEvaluateDKappa(self):
        "tests vonMisesSufficient.evaluate_derivative_kappa"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,cos(randno[1]),sin(randno[1]),randno[2])
            self.assertAlmostEqual(fn.evaluate_derivative_kappa(),
                    fn2.evaluate_derivative_kappa(),
                    delta=0.001)

    def testDensity(self):
        "tests vonMisesSufficient.density"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,cos(randno[1]),sin(randno[1]),randno[2])
            self.assertAlmostEqual(fn.density(),
                    fn2.density(),
                    delta=0.001)

if __name__ == '__main__':
    IMP.test.main()




        
        
        




        

