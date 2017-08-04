#!/usr/bin/env python
from __future__ import division
from itertools import product

import IMP
from IMP.isd import Nuisance, Scale, StudentTRestraint, FStudentT
import IMP.test

try:
    import scipy.special
except ImportError:
    scipy = None
import numpy as np


def evaluate(x, m, s, v):
    return (scipy.special.gammaln(.5 * v) -
            scipy.special.gammaln(.5 * (v + 1)) + .5 * np.log(np.pi * v) +
            np.log(s) + .5 * (v + 1) * np.log(1 + ((x - m) / s)**2 / v))


def evaluate_dx(x, m, s, v):
    return (v + 1) * (x - m) / (s**2 * v + (x - m)**2)


def evaluate_dm(x, m, s, v):
    return -evaluate_dx(x, m, s, v)


def evaluate_ds(x, m, s, v):
    return v * (s + m - x) * (s + x - m) / s / (s**2 * v + (m - x)**2)


def evaluate_dv(x, m, s, v):
    return .5 * ((1 - (v + 1) * (x - m)**2 / (s**2 * v + (m - x)**2)) / v +
                 np.log(1 + (x - m)**2 / s**2 / v) +
                 (scipy.special.digamma(.5 * v) -
                  scipy.special.digamma(.5 * (v + 1))))


class TestCase(IMP.test.TestCase):

    """Tests for StudentTRestraint."""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        if scipy is None:
            self.skipTest("No scipy.special module")

    def create_particles(self):
        x = Nuisance.setup_particle(IMP.Particle(self.m), 2.0)
        mu = Nuisance.setup_particle(IMP.Particle(self.m), 1.0)
        sigma = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        nu = Scale.setup_particle(IMP.Particle(self.m), 3.0)
        all_nuis = [x, mu, sigma, nu]
        return all_nuis

    def test_all_setup_combinations(self):
        """Test all constructors work without error."""
        all_nuis = self.create_particles()
        it = product(*[[x, x.get_nuisance()] for x in all_nuis])
        for (x, mu, sigma, nu) in it:
            if all([isinstance(v, float) for v in [x, mu, sigma,
                                                   nu]]):
                continue
            StudentTRestraint(self.m, x, mu, sigma, nu)

    def test_evaluate(self):
        """Test correct score is produced."""
        all_nuis = self.create_particles()
        r = StudentTRestraint(self.m, *all_nuis)
        exp_score = evaluate(*[x.get_nuisance() for x in all_nuis])
        score = r.evaluate(False)
        self.assertAlmostEqual(score, exp_score, delta=1e-6)

    def test_derivative_dx(self):
        """Test derivative wrt x."""
        all_nuis = self.create_particles()
        r = StudentTRestraint(self.m, *all_nuis)
        exp_dx = evaluate_dx(*[x.get_nuisance() for x in all_nuis])
        r.evaluate(True)
        self.assertAlmostEqual(all_nuis[0].get_nuisance_derivative(), exp_dx)

    def test_derivative_dmu(self):
        """Test derivative wrt mu."""
        all_nuis = self.create_particles()
        r = StudentTRestraint(self.m, *all_nuis)
        exp_dmu = evaluate_dm(*[x.get_nuisance() for x in all_nuis])
        r.evaluate(True)
        self.assertAlmostEqual(all_nuis[1].get_nuisance_derivative(), exp_dmu)

    def test_derivative_dsigma(self):
        """Test derivative wrt sigma."""
        all_nuis = self.create_particles()
        r = StudentTRestraint(self.m, *all_nuis)
        exp_dsigma = evaluate_ds(*[x.get_nuisance() for x in all_nuis])
        r.evaluate(True)
        self.assertAlmostEqual(all_nuis[2].get_nuisance_derivative(),
                               exp_dsigma)

    def test_derivative_dnu(self):
        """Test derivative wrt nu."""
        all_nuis = self.create_particles()
        r = StudentTRestraint(self.m, *all_nuis)
        exp_dnu = evaluate_dv(*[x.get_nuisance() for x in all_nuis])
        r.evaluate(True)
        self.assertAlmostEqual(all_nuis[3].get_nuisance_derivative(), exp_dnu)


if __name__ == '__main__':
    IMP.test.main()
