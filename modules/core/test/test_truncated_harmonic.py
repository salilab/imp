import unittest
import IMP
import IMP.test
import IMP.core
import random

class LinearTests(IMP.test.TestCase):
    """Tests for linear unary function"""

    def _test_one(self, f, m):
        self.check_unary_function_deriv(f, -30, 30, .1)

    def test_lvalues(self):
        """Check TruncatedHarmonicLowerBound"""
        c= random.uniform(-10, 10)
        k= random.uniform(.1, 10)
        t= random.uniform(0,10)
        l= random.uniform(1.1*.5*k*t*t, 10+1.1*.5*k*t*t)
        print "c=" +str(c)
        print "k=" +str(k)
        print "t=" +str(t)
        print "l=" +str(l)
        h= IMP.core.TruncatedHarmonicLowerBound(c, k, t, l)
        self.check_unary_function_deriv(h, -10, 10, .1)
        self.assertEqual(h.evaluate(c+1), 0)
        self.assertInTolerance(h.evaluate(c-t-1000), l, .1)

    def test_uvalues(self):
        """Check TruncatedHarmonicUpperBound"""
        c= random.uniform(-10, 10)
        k= random.uniform(.1, 10)
        t= random.uniform(0,10)
        l= random.uniform(1.1*.5*k*t*t, 10+1.1*.5*k*t*t)
        c=0
        k=1
        t=2
        l=10
        print "c=" +str(c)
        print "k=" +str(k)
        print "t=" +str(t)
        print "l=" +str(l)
        h= IMP.core.TruncatedHarmonicUpperBound(c, k, t, l)
        self.check_unary_function_deriv(h, -10, 10, .1)
        self.assertEqual(h.evaluate(c-1), 0)
        self.assertInTolerance(h.evaluate(c+t+1000), l, .1)

    def test_values(self):
        """Check TruncatedHarmonicBound"""
        c= random.uniform(-10, 10)
        k= random.uniform(.1, 10)
        t= random.uniform(0,10)
        l= random.uniform(1.1*.5*k*t*t, 10+1.1*.5*k*t*t)
        #c=0
        #k=1
        #t=2
        #l=10
        print "c=" +str(c)
        print "k=" +str(k)
        print "t=" +str(t)
        print "l=" +str(l)
        h= IMP.core.TruncatedHarmonicBound(c, k, t, l)
        for x in range(-200,200):
            print str(x/10.0) +", "+str(h.evaluate(x/10.0))

        self.assertInTolerance(h.evaluate(c+t+1000), l, .1)
        self.assertInTolerance(h.evaluate(c-t-1000), l, .1)
        self.check_unary_function_min(h, -10, 10, .1, c)
        self.check_unary_function_deriv(h, -10, 10, .1)

if __name__ == '__main__':
    unittest.main()
