#!/usr/bin/env python

# general imports
from numpy import *
from random import uniform
import pickle

# imp general
import IMP

# our project
from IMP.isd import Scale, JeffreysRestraint, AmbiguousNOERestraint

# unit testing framework
import IMP.test


class Tests(IMP.test.TestCase):

    "simple test cases to check if AmbiguousNOERestraint works"

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.gamma = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.p0 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                                              IMP.algebra.Vector3D(0, 0, 0))
        self.p1 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                                              IMP.algebra.Vector3D(1, 1, 1))
        self.p2 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                                              IMP.algebra.Vector3D(1, 0, 0))
        self.DA = IMP.DerivativeAccumulator()
        self.V_obs = 3.0
        self.ls = \
            IMP.container.ListPairContainer(self.m,
                [(self.p0, self.p1), (self.p0, self.p2)])
        self.noe = IMP.isd.AmbiguousNOERestraint(self.m, self.ls, self.sigma,
                                                 self.gamma, self.V_obs)

    def testValuePDist1(self):
        "Test AmbiguousNOERestraint probability by changing distance 1"
        # Probability should be equal to
        # 1/(sqrt(2*pi)*sigma*V_obs) *
        # exp(-1/(2sigma^2)*log^2(gamma*d^-6/Vobs))
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = self.sigma.get_scale()
            gamma = self.gamma.get_scale()
            p1.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1, 100)
                                                      for i in range(3)]))
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 1 / (sqrt(2 * pi) * no * self.V_obs) * exp(
                -1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2)
            self.assertAlmostEqual(self.noe.get_probability(),
                                   expected, delta=0.001)

    def testValuePDist2(self):
        "Test AmbiguousNOERestraint probability by changing distance 2"
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = self.sigma.get_scale()
            gamma = self.gamma.get_scale()
            p2.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1, 100)
                                                      for i in range(3)]))
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 1 / (sqrt(2 * pi) * no * self.V_obs) * exp(
                -1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2)
            self.assertAlmostEqual(self.noe.get_probability(),
                                   expected, delta=0.001)

    def testValuePSigma(self):
        "Test AmbiguousNOERestraint probability by changing sigma"
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            gamma = self.gamma.get_scale()
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 1 / (sqrt(2 * pi) * no * self.V_obs) * exp(
                -1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2)
            self.assertAlmostEqual(self.noe.get_probability(),
                                   expected, delta=0.001)

    def testValuePGamma(self):
        "Test AmbiguousNOERestraint probability by changing gamma"
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = self.sigma.get_scale()
            gamma = uniform(0.1, 100)
            self.gamma.set_scale(gamma)
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 1 / (sqrt(2 * pi) * no * self.V_obs) * exp(
                -1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2)
            self.assertAlmostEqual(self.noe.get_probability(),
                                   expected, delta=0.001)

    def testValueEDist(self):
        "Test AmbiguousNOERestraint score by changing position of p0"
        # Score should be equal to
        # 0.5*log(2*pi) + log(sigma*V_obs) +
        # 1/(2sigma^2)*log^2(gamma*d^-6/Vobs)
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = self.sigma.get_scale()
            gamma = self.gamma.get_scale()
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1, 100)
                                                      for i in range(3)]))
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 0.5 * log(2 * pi) + log(no * self.V_obs) + \
                1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2
            self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                                   expected, delta=0.001)

    def testValueChi(self):
        "Test AmbiguousNOERestraint chi by changing position of p0"
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = self.sigma.get_scale()
            gamma = self.gamma.get_scale()
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1, 100)
                                                      for i in range(3)]))
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = -log(gamma * dist ** -6 / self.V_obs)
            self.noe.unprotected_evaluate(None)
            self.assertAlmostEqual(self.noe.get_chi(),
                                   expected, delta=0.001)

    def testValueESigma(self):
        "Test AmbiguousNOERestraint score by changing sigma"
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            gamma = self.gamma.get_scale()
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 0.5 * log(2 * pi) + log(no * self.V_obs) + \
                1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2
            self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                                   expected, delta=0.001)

    def testValueEGamma(self):
        "Test AmbiguousNOERestraint score by changing gamma"
        for i in range(100):
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            no = self.sigma.get_scale()
            gamma = uniform(0.1, 100)
            self.gamma.set_scale(gamma)
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            expected = 0.5 * log(2 * pi) + log(no * self.V_obs) + \
                1 / (2 * no ** 2) * log(gamma * dist ** -6 / self.V_obs) ** 2
            self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                                   expected, delta=0.001)

    def test_inputs(self):
        "Test AmbiguousNOERestraint::get_inputs"
        self.assertEqual([x.get_name() for x in self.noe.get_inputs()],
                         [y.get_name() for y in
                          [self.p0, self.p1, self.p0, self.p2, self.sigma,
                           self.gamma, self.ls]])

    def testDerivativeX(self):
        "Test AmbiguousNOERestraint derivative w/r to X"
        sf = IMP.core.RestraintsScoringFunction([self.noe])
        for i in range(100):
            pos0 = [uniform(0.1, 100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1 = [uniform(0.1, 100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            pos2 = [uniform(0.1, 100) for i in range(3)]
            self.p2.set_coordinates(IMP.algebra.Vector3D(*pos2))
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            sigma = uniform(0.1, 100)
            self.sigma.set_scale(sigma)
            gamma = uniform(0.1, 100)
            self.gamma.set_scale(gamma)
            sf.evaluate(True)
            for coord in range(3):
                self.assertAlmostEqual(self.p0.get_derivative(coord),
                                       ((pos0[coord] - pos1[coord]) * 6 * dist1 ** (-8)
                                        + (pos0[coord] - pos2[coord]) * 6 * dist2 ** (-8))
                                       * (1 / sigma ** 2 * log(self.V_obs / (gamma * dist ** -6))) / dist ** (-6),
                                       delta=0.001)
                self.assertAlmostEqual(self.p1.get_derivative(coord),
                                       (pos1[coord] - pos0[coord]) * 6 * dist1 ** (
                                           -8)
                                       * (1 / sigma ** 2 * log(self.V_obs / (gamma * dist ** -6))) / dist ** (-6),
                                       delta=0.001)
                self.assertAlmostEqual(self.p2.get_derivative(coord),
                                       (pos2[coord] - pos0[coord]) * 6 * dist2 ** (
                                           -8)
                                       * (1 / sigma ** 2 * log(self.V_obs / (gamma * dist ** -6))) / dist ** (-6),
                                       delta=0.001)

    def testDerivativeSigma(self):
        "Test AmbiguousNOERestraint derivative w/r to sigma"
        sf = IMP.core.RestraintsScoringFunction([self.noe])
        for i in range(100):
            pos0 = [uniform(0.1, 100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1 = [uniform(0.1, 100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            pos2 = [uniform(0.1, 100) for i in range(3)]
            self.p2.set_coordinates(IMP.algebra.Vector3D(*pos2))
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            sigma = uniform(0.1, 100)
            self.sigma.set_scale(sigma)
            gamma = uniform(0.1, 100)
            self.gamma.set_scale(gamma)
            sf.evaluate(True)
            self.assertAlmostEqual(self.sigma.get_scale_derivative(),
                                   1 / sigma - 1 / sigma ** 3 *
                                   log(self.V_obs / (gamma * dist ** -6)) ** 2,
                                   delta=0.001)

    def testDerivativeGamma(self):
        "Test AmbiguousNOERestraint derivative w/r to gamma"
        sf = IMP.core.RestraintsScoringFunction([self.noe])
        for i in range(100):
            pos0 = [uniform(0.1, 100) for i in range(3)]
            self.p0.set_coordinates(IMP.algebra.Vector3D(*pos0))
            pos1 = [uniform(0.1, 100) for i in range(3)]
            self.p1.set_coordinates(IMP.algebra.Vector3D(*pos1))
            pos2 = [uniform(0.1, 100) for i in range(3)]
            self.p2.set_coordinates(IMP.algebra.Vector3D(*pos2))
            p0 = self.p0
            p1 = self.p1
            p2 = self.p2
            dist1 = IMP.core.get_distance(p0, p1)
            dist2 = IMP.core.get_distance(p0, p2)
            dist = (dist1 ** -6 + dist2 ** -6) ** (-1. / 6)
            sigma = uniform(0.1, 100)
            self.sigma.set_scale(sigma)
            gamma = uniform(0.1, 100)
            self.gamma.set_scale(gamma)
            sf.evaluate(True)
            self.assertAlmostEqual(self.gamma.get_scale_derivative(),
                                   1 / gamma *
                                   (-1 / sigma ** 2 * log(
                                       self.V_obs / (gamma * dist ** -6))),
                                   delta=0.001)

    def testSanityEP(self):
        "Test if AmbiguousNOERestraint score is -log(prob)"
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.noe.unprotected_evaluate(self.DA),
                                   -log(self.noe.get_probability()), delta=0.001)

    def testSanityPE(self):
        "Test if AmbiguousNOERestraint prob is exp(-score)"
        for i in range(100):
            no = uniform(0.1, 100)
            self.sigma.set_scale(no)
            self.assertAlmostEqual(self.noe.get_probability(),
                                   exp(-self.noe.unprotected_evaluate(self.DA)), delta=0.001)

    def test_serialize(self):
        """Test (un-)serialize of AmbiguousNOERestraint"""
        self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                               2.8517, delta=1e-4)
        self.noe.set_name("foo")
        dump = pickle.dumps(self.noe)
        newnoe = pickle.loads(dump)
        self.assertAlmostEqual(newnoe.unprotected_evaluate(None),
                               2.8517, delta=1e-4)
        self.assertEqual(newnoe.get_name(), "foo")

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of AmbiguousNOERestraint via polymorphic ptr"""
        self.assertAlmostEqual(self.noe.unprotected_evaluate(None),
                               2.8517, delta=1e-4)
        self.noe.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([self.noe])
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newnoe, = newsf.restraints
        self.assertAlmostEqual(newnoe.unprotected_evaluate(None),
                               2.8517, delta=1e-4)
        self.assertEqual(newnoe.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
