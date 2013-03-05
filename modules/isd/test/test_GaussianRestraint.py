#!/usr/bin/env python

#general imports
from numpy import *


#imp general
import IMP

#our project
from IMP.isd import Nuisance,Scale,GaussianRestraint

#unit testing framework
import IMP.test

class TestGaussianRestraintSimple3(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.mu = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.x = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_mu(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (mu-x)/sigma**2

    def deriv_x(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (x-mu)/sigma**2

    def deriv_sigma(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return -(x-mu)**2/sigma**3 + 1./sigma

    def testE(self):
        "Test GaussianRestraint(3) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdx(self):
        "Test GaussianRestraint(3) x derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.x).get_nuisance_derivative(),
                    self.deriv_x(*self.all))

    def testdmu(self):
        "Test GaussianRestraint(3) mu derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.mu).get_nuisance_derivative(),
                    self.deriv_mu(*self.all))

    def testdsigma(self):
        "Test GaussianRestraint(3) sigma derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(
                    Nuisance(self.sigma).get_nuisance_derivative(),
                    self.deriv_sigma(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(3) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(3) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

    def testFail(self):
        "Test failures of GaussianRestraint(3)"
        if IMP.base.get_check_level() >= IMP.base.USAGE:
            dummy=IMP.Particle(self.m)
            self.assertRaises(IMP.base.UsageException, GaussianRestraint, dummy,
                              self.all[1], self.all[2])
            self.assertRaises(IMP.base.UsageException, GaussianRestraint,
                              self.all[0], dummy, self.all[2])
            self.assertRaises(IMP.base.UsageException, GaussianRestraint,
                              self.all[0], self.all[1], dummy)

class TestGaussianRestraintSimple21(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = 2.0
        self.mu = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.x = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_mu(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (mu-x)/sigma**2

    def deriv_x(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (x-mu)/sigma**2

    def testE(self):
        "Test GaussianRestraint(21) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdx(self):
        "Test GaussianRestraint(21) x derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.x).get_nuisance_derivative(),
                    self.deriv_x(*self.all))

    def testdmu(self):
        "Test GaussianRestraint(21) mu derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.mu).get_nuisance_derivative(),
                    self.deriv_mu(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(21) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(21) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

class TestGaussianRestraintSimple22(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.mu = 1.0
        self.x = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_x(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (x-mu)/sigma**2

    def deriv_sigma(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return -(x-mu)**2/sigma**3 + 1./sigma

    def testE(self):
        "Test GaussianRestraint(22) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdx(self):
        "Test GaussianRestraint(22) x derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.x).get_nuisance_derivative(),
                    self.deriv_x(*self.all))

    def testdsigma(self):
        "Test GaussianRestraint(22) sigma derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(
                    Nuisance(self.sigma).get_nuisance_derivative(),
                    self.deriv_sigma(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(22) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(22) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

class TestGaussianRestraintSimple23(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.mu = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.x = 2.0
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_mu(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (mu-x)/sigma**2

    def deriv_sigma(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return -(x-mu)**2/sigma**3 + 1./sigma

    def testE(self):
        "Test GaussianRestraint(23) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdmu(self):
        "Test GaussianRestraint(23) mu derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.mu).get_nuisance_derivative(),
                    self.deriv_mu(*self.all))

    def testdsigma(self):
        "Test GaussianRestraint(23) sigma derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(
                    Nuisance(self.sigma).get_nuisance_derivative(),
                    self.deriv_sigma(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(23) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(23) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

class TestGaussianRestraintSimple11(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.mu = 1.0
        self.x = 2.0
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_sigma(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return -(x-mu)**2/sigma**3 + 1./sigma

    def testE(self):
        "Test GaussianRestraint(11) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdsigma(self):
        "Test GaussianRestraint(11) sigma derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(
                    Nuisance(self.sigma).get_nuisance_derivative(),
                    self.deriv_sigma(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(11) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(11) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

class TestGaussianRestraintSimple12(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = 2.0
        self.mu = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        self.x = 2.0
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_mu(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (mu-x)/sigma**2

    def testE(self):
        "Test GaussianRestraint(12) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdmu(self):
        "Test GaussianRestraint(12) mu derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.mu).get_nuisance_derivative(),
                    self.deriv_mu(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(12) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(12) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

class TestGaussianRestraintSimple13(IMP.test.TestCase):
    "simple test cases to check if GaussianRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = 2.0
        self.mu = 1.0
        self.x = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        self.locations=[self.x, self.mu]
        self.all = self.locations+[self.sigma]
        self.DA = IMP.DerivativeAccumulator()

    def get_value(self, p):
        try:
            v = Nuisance(p).get_nuisance()
        except:
            v = p
        return v

    def change_value(self, p, min=0.1, max=100):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(random.uniform(min,max))

    def change_sign(self, p):
        try:
            n = Nuisance(p)
        except:
            return
        n.set_nuisance(n.get_nuisance()*(2*random.randint(2)-1))

    def normal_p(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 1/(sqrt(2*pi)*sigma) * exp(-1/(2*sigma**2)*(x-mu)**2)

    def normal_e(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return 0.5*log(2*pi) + log(sigma) + 1/(2*sigma**2)*(x-mu)**2

    def deriv_x(self, x, mu, sigma):
        (x,mu,sigma) = map(self.get_value, [x,mu,sigma])
        return (x-mu)/sigma**2

    def testE(self):
        "Test GaussianRestraint(13) score"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            e=self.m.evaluate(False)
            self.assertAlmostEqual(e, self.normal_e(*self.all))

    def testdx(self):
        "Test GaussianRestraint(13) x derivative"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.m.evaluate(True)
            self.assertAlmostEqual(Nuisance(self.x).get_nuisance_derivative(),
                    self.deriv_x(*self.all))

    def testSanityPE(self):
        "Test if GaussianRestraint(13) prob is exp(-score)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            self.assertAlmostEqual(gr.get_probability(),
                    exp(-self.m.evaluate(False)),delta=0.001)

    def testSanityEP(self):
        "Test if GaussianRestraint(13) score is -log(prob)"
        gr=GaussianRestraint(*self.all)
        self.m.add_restraint(gr)
        for i in xrange(100):
            map(self.change_value, self.all)
            map(self.change_sign, self.locations)
            expected = gr.get_probability()
            if expected == 0:
                continue
            self.assertAlmostEqual(-log(expected),
                    self.m.evaluate(False),delta=0.6)

if __name__ == '__main__':
    IMP.test.main()
