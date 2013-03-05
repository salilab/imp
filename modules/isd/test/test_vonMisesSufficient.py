#!/usr/bin/env python

#general imports
from math import *
from numpy import *
from random import uniform,randint,gauss

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
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)

    def testAlternative(self):
        "Test alternative vonMisesSufficient constructor"
        for i in xrange(100):
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            meanv=uniform(-pi,pi)
            stdev=uniform(0,2*pi)
            obs = array([gauss(meanv,stdev) for j in xrange(N)])
            cosbar = cos(obs).sum()
            sinbar = sin(obs).sum()
            R=sqrt(cosbar**2+sinbar**2)
            chiexp=acos(cosbar/R)
            if sinbar <0:
                chiexp = -chiexp
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,R,chiexp,kappa)
            fn2=vonMisesSufficient(x,obs,kappa)
            self.assertAlmostEqual(fn.evaluate(), fn2.evaluate(), delta=1e-6)

    def testStatistics(self):
        "Test vonMisesSufficient statistics"
        for i in xrange(100):
            N=randint(1,20)
            meanv=uniform(-pi,pi)
            stdev=uniform(0,2*pi)
            obs = array([gauss(meanv,stdev) for j in xrange(N)])
            cpp = IMP.isd.vonMisesSufficient.get_sufficient_statistics(obs)
            cosbar = cos(obs).sum()
            sinbar = sin(obs).sum()
            R=sqrt(cosbar**2+sinbar**2)
            chiexp=acos(cosbar/R)
            if sinbar <0:
                chiexp = -chiexp
            self.assertEqual(cpp[0],N)
            self.assertAlmostEqual(cpp[1],R,delta=1e-6)
            self.assertAlmostEqual(cpp[2],chiexp,delta=1e-6)

    def testEvaluate(self):
        "Test vonMisesSufficient.evaluate"
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        for i in xrange(100):
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            R=randint(1,N)
            chiexp=uniform(-pi,pi)
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,R,chiexp,kappa)
            self.assertAlmostEqual(fn.evaluate(),
                    log(2*pi*i0(kappa)**N) - R*kappa*cos(x-chiexp),
                    delta=0.001)

    def testEvaluateDX(self):
        "Test vonMisesSufficient.evaluate_derivative_x"
        for i in xrange(100):
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            R=randint(1,N)
            chiexp=uniform(-pi,pi)
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,R,chiexp,kappa)
            self.assertAlmostEqual(fn.evaluate_derivative_x(),
                    R*kappa*sin(x-chiexp),
                    delta=0.001)

    def testEvaluateDKappa(self):
        "Test vonMisesSufficient.evaluate_derivative_kappa"
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        for i in xrange(100):
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            R=randint(1,N)
            chiexp=uniform(-pi,pi)
            kappa=uniform(0.1,100)
            fn=vonMisesSufficient(x,N,R,chiexp,kappa)
            self.assertAlmostEqual(fn.evaluate_derivative_kappa(),
                    N*i1(kappa)/i0(kappa) - R*cos(x-chiexp),
                    delta=0.001)

    def testDensity(self):
        "Test vonMisesSufficient.density"
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        for i in xrange(100):
            #x, N, cmu1, smu2, kappa
            x=uniform(-4*pi,4*pi)
            N=randint(1,20)
            R=randint(1,N)
            chiexp=uniform(-pi,pi)
            kappa=uniform(0.1,10)
            fn=vonMisesSufficient(x,N,R,chiexp,kappa)
            cpp=fn.density()
            py=exp(R*kappa*cos(x-chiexp))/(2*pi*i0(kappa)**N)
            if py == 0.0:
                self.assertAlmostEqual(cpp,0.0,delta=0.001)
            else:
                self.assertAlmostEqual(cpp/py,1.0,delta=0.001)

class TestvonMisesSufficientDegenerate(IMP.test.TestCase):
    """the sufficient von Mises should reduce to the von Mises
    when N=R=1
    """

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)

    def testEvaluate(self):
        "Test vonMisesSufficient.evaluate degenerate case"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,1,randno[1],randno[2])
            self.assertAlmostEqual(fn.evaluate(), fn2.evaluate(), delta=0.001)

    def testEvaluateDX(self):
        "Test vonMisesSufficient.evaluate_derivative_x degenerate case"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,1,randno[1],randno[2])
            self.assertAlmostEqual(fn.evaluate_derivative_x(),
                    fn2.evaluate_derivative_x(),
                    delta=0.001)

    def testEvaluateDKappa(self):
        "Test vonMisesSufficient.evaluate_derivative_kappa degenerate case"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,1,randno[1],randno[2])
            self.assertAlmostEqual(fn.evaluate_derivative_kappa(),
                    fn2.evaluate_derivative_kappa(),
                    delta=0.001)

    def testDensity(self):
        "Test vonMisesSufficient.density degenerate case"
        for i in xrange(100):
            randno = [uniform(-4*pi,4*pi), uniform(-pi,pi),
                    uniform(0.1,100)]
            fn=vonMises(*randno)
            fn2=vonMisesSufficient(randno[0],1,1,randno[1],randno[2])
            self.assertAlmostEqual(fn.density(),
                    fn2.density(),
                    delta=0.001)

if __name__ == '__main__':
    IMP.test.main()
