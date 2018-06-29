#!/usr/bin/env python
from __future__ import division
import math
import random

import IMP
from IMP.isd import PenalizedComplexityPrior
import IMP.test


def get_normalized_kld(tau, sigmau):
    return 1. / sigmau**2 / tau


def get_derivative_wrt_normalized_kld(tau, sigmau):
    return -1. / sigmau**2 / tau**2


def get_second_derivative_wrt_normalized_kld(tau, sigmau):
    return 2. / sigmau**2 / tau**3


def compute_lambda(alpha, sigmau):
    return -math.log(alpha) / sigmau


def evaluate_type2_gumbel(tau, lam):
    return math.log(2. / lam) + 1.5 * math.log(tau) + lam / math.sqrt(tau)


def evaluate_type2_gumbel_derivative_tau(tau, lam):
    return 1.5 / tau - .5 * lam / tau**1.5


class Tests(IMP.test.TestCase):

    """Tests for PenalizedComplexityPrior."""

    def test_create_prior(self):
        pc = PenalizedComplexityPrior(1, -2, .01)
        self.assertAlmostEqual(pc.get_Dxi(), 1)
        self.assertAlmostEqual(pc.get_Jxi(), -2)
        self.assertAlmostEqual(pc.get_alpha(), .01)
        pc.set_was_used(True)

    def test_invalid_alpha_raises_usage_error(self):
        self.assertRaisesUsageException(PenalizedComplexityPrior, 1, 1, 0)
        self.assertRaisesUsageException(PenalizedComplexityPrior, 1, 1, 1)
        pc = PenalizedComplexityPrior(1, 1, .01)

        self.assertRaisesUsageException(pc.set_alpha, 0)
        self.assertRaisesUsageException(pc.set_alpha, 1)
        pc.set_was_used(True)

    def test_invalid_Dxi_raises_usage_error(self):
        self.assertRaisesUsageException(PenalizedComplexityPrior, 0, 1, .01)
        self.assertRaisesUsageException(PenalizedComplexityPrior, -1, 1, .01)
        pc = PenalizedComplexityPrior(1, 1, .01)

        self.assertRaisesUsageException(pc.set_Dxi, 0)
        self.assertRaisesUsageException(pc.set_Dxi, -1)
        pc.set_was_used(True)

    def test_update_sufficient_statistics(self):
        """Test sufficient statistics correctly updated."""
        pc = PenalizedComplexityPrior(1, -1, .01)
        pc.update_sufficient_statistics([10], [2])
        stats = pc.get_sufficient_statistics()
        self.assertAlmostEqual(stats[0], 10)
        self.assertAlmostEqual(stats[1], 2)
        pc.set_was_used(True)

    def test_setters_getters(self):
        pc = PenalizedComplexityPrior(1, 2, .01)
        self.assertAlmostEqual(pc.get_Dxi(), 1)
        self.assertAlmostEqual(pc.get_Jxi(), 2)
        self.assertAlmostEqual(pc.get_alpha(), .01)

        pc.set_Dxi(10)
        pc.set_Jxi(-1)
        pc.set_alpha(1e-5)
        self.assertAlmostEqual(pc.get_Dxi(), 10)
        self.assertAlmostEqual(pc.get_Jxi(), -1)
        self.assertAlmostEqual(pc.get_alpha(), 1e-5)
        pc.set_was_used(True)

    def test_evaluate(self):
        """Test PenalizedComplexityPrior.evaluate"""
        n = 10
        for i in range(n):
            t = random.uniform(0, 500)
            a = random.uniform(0, 1)
            su = random.uniform(1, 10)
            dt = get_normalized_kld(t, su)
            jt = get_derivative_wrt_normalized_kld(t, su)
            lam = compute_lambda(a, su)
            exp_score = evaluate_type2_gumbel(t, lam)
            pc = PenalizedComplexityPrior(dt, jt, a)
            self.assertAlmostEqual(pc.evaluate(), exp_score, delta=1e-6)
            pc.set_was_used(True)

    def test_get_density(self):
        """Test PenalizedComplexityPrior.get_density"""
        n = 10
        for i in range(n):
            t = random.uniform(0, 500)
            a = random.uniform(0, 1)
            su = random.uniform(1, 10)
            dt = get_normalized_kld(t, su)
            jt = get_derivative_wrt_normalized_kld(t, su)
            lam = compute_lambda(a, su)
            exp_dens = math.exp(-evaluate_type2_gumbel(t, lam))
            pc = PenalizedComplexityPrior(dt, jt, a)
            self.assertAlmostEqual(pc.get_density(), exp_dens, delta=1e-6)
            pc.set_was_used(True)

    def test_evaluate_DDxi(self):
        """Test PenalizedComplexityPrior.evaluate_derivative_Dxi"""
        n = 10
        for i in range(n):
            t = random.uniform(0, 500)
            a = random.uniform(0, 1)
            su = random.uniform(1, 10)
            dt = get_normalized_kld(t, su)
            jt = get_derivative_wrt_normalized_kld(t, su)
            exp_derv = .5 * (1. / dt - math.log(a) / dt**.5)
            pc = PenalizedComplexityPrior(dt, jt, a)
            self.assertAlmostEqual(pc.evaluate_derivative_Dxi(), exp_derv,
                                   delta=1e-6)
            pc.set_was_used(True)

    def test_evaluate_DJxi(self):
        """Test PenalizedComplexityPrior.evaluate_derivative_Jxi"""
        n = 10
        for i in range(n):
            t = random.uniform(0, 500)
            a = random.uniform(0, 1)
            su = random.uniform(1, 10)
            dt = get_normalized_kld(t, su)
            jt = get_derivative_wrt_normalized_kld(t, su)
            exp_derv = -1. / jt
            pc = PenalizedComplexityPrior(dt, jt, a)
            self.assertAlmostEqual(pc.evaluate_derivative_Jxi(), exp_derv,
                                   delta=1e-6)
            pc.set_Jxi(-jt)
            self.assertAlmostEqual(pc.evaluate_derivative_Jxi(), -exp_derv,
                                   delta=1e-6)
            pc.set_was_used(True)

    def test_evaluate_Dalpha(self):
        """Test PenalizedComplexityPrior.evaluate_derivative_alpha"""
        n = 10
        for i in range(n):
            t = random.uniform(0, 500)
            a = random.uniform(0, 1)
            su = random.uniform(1, 10)
            dt = get_normalized_kld(t, su)
            jt = get_derivative_wrt_normalized_kld(t, su)
            exp_derv = -1. / a * (math.sqrt(dt) + 1. / math.log(a))
            pc = PenalizedComplexityPrior(dt, jt, a)
            self.assertAlmostEqual(pc.evaluate_derivative_alpha(), exp_derv,
                                   delta=1e-6)
            pc.set_was_used(True)

    def test_evaluate_Dxi(self):
        """Test PenalizedComplexityPrior.evaluate_derivative_xi"""
        n = 10
        for i in range(n):
            t = random.uniform(0, 500)
            a = random.uniform(0, 1)
            su = random.uniform(1, 10)
            dt = get_normalized_kld(t, su)
            jt = get_derivative_wrt_normalized_kld(t, su)
            jjt = get_second_derivative_wrt_normalized_kld(t, su)
            lam = compute_lambda(a, su)
            exp_derv = evaluate_type2_gumbel_derivative_tau(t, lam)
            pc = PenalizedComplexityPrior(dt, jt, a)
            self.assertAlmostEqual(pc.evaluate_derivative_xi(jjt), exp_derv,
                                   delta=1e-6)
            pc.set_was_used(True)


if __name__ == '__main__':
    IMP.test.main()
