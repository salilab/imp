#!/usr/bin/env python
from __future__ import division
from random import uniform

import numpy as np
from scipy.special import gammaln, digamma
import IMP
from IMP.isd import FStudentT
import IMP.test


def evaluate_fstudentt(fxs, jxs, fm, s, v):
    n = float(fxs.size)
    sumfx = np.sum(fxs)
    sumfx2 = np.sum(fxs**2)
    logjx = np.sum(np.log(jxs))
    t2 = (sumfx2 - 2 * fm * sumfx + n * fm**2) / s**2
    return (gammaln(.5 * v) - gammaln(.5 * (n + v)) +
            .5 * n * np.log(np.pi * v) + n * np.log(s) +
            .5 * (n + v) * np.log(1 + t2 / v) - logjx)


def evaluate_studentt(xs, fm, s, v):
    return evaluate_fstudentt(xs, np.ones_like(xs), fm, s, v)


def evaluate_logstudentt(logxs, logm, s, v):
    return evaluate_fstudentt(logxs, np.exp(-logxs), logm, s, v)


def evaluate_derivative_fm(fxs, fm, s, v):
    n = float(fxs.size)
    sumfx = np.sum(fxs)
    sumfx2 = np.sum(fxs**2)
    t2 = (sumfx2 - 2 * fm * sumfx + n * fm**2) / s**2
    dt2fm = 2 * (n * fm - sumfx) / s**2
    return (n + v) * dt2fm / (2 * (v + t2))


def evaluate_derivative_s(fxs, fm, s, v):
    n = float(fxs.size)
    sumfx = np.sum(fxs)
    sumfx2 = np.sum(fxs**2)
    t2 = (sumfx2 - 2 * fm * sumfx + n * fm**2) / s**2
    dt2s = -2 * t2 / s
    return n / s + (n + v) * dt2s / (2 * (v + t2))


def evaluate_derivative_v(fxs, fm, s, v):
    n = float(fxs.size)
    sumfx = np.sum(fxs)
    sumfx2 = np.sum(fxs**2)
    t2 = (sumfx2 - 2 * fm * sumfx + n * fm**2) / s**2
    return .5 * (-1 + digamma(.5 * v) - digamma(.5 * (n + v)) +
                 np.log(1 + t2 / v) + (n + v) / (v + t2))


class Tests(IMP.test.TestCase):

    """Tests for FStudentT."""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)

    def test_update_sufficient_statistics(self):
        """Test sufficient statistics correctly updated."""
        n = 10
        for i in range(10):
            x = np.random.uniform(-100, 100, n)
            jx = 2 * np.ones_like(x)
            sumx = x.sum()
            sumx2 = (x**2).sum()
            logJX = np.log(jx).sum()
            f = FStudentT(x, jx, 1., 10., 10., "test")
            stats = f.get_sufficient_statistics()
            self.assertAlmostEqual(stats[0], sumx, delta=1e-4)
            self.assertAlmostEqual(stats[1], sumx2, delta=1e-2)
            self.assertAlmostEqual(stats[2], n, delta=1e-6)
            self.assertAlmostEqual(f.get_LogJX(), logJX, delta=1e-6)

            x = np.random.uniform(-100, 100, n)
            jx = 3 * np.ones_like(x)
            sumx = x.sum()
            sumx2 = (x**2).sum()
            logJX = np.log(jx).sum()
            f.update_sufficient_statistics(x, jx)
            stats = f.get_sufficient_statistics()
            self.assertAlmostEqual(stats[0], sumx, delta=1e-4)
            self.assertAlmostEqual(stats[1], sumx2, delta=1e-2)
            self.assertAlmostEqual(stats[2], n, delta=1e-6)
            self.assertAlmostEqual(f.get_LogJX(), logJX, delta=1e-6)

    def test_create_from_sufficient_statistics(self):
        """Test constructor with sufficient stats."""
        n = 10
        for i in range(10):
            x = np.random.uniform(-100, 100, n)
            jx = 2 * np.ones_like(x)
            sumx = x.sum()
            sumx2 = (x**2).sum()
            logJX = np.log(jx).sum()
            f = FStudentT(sumx, sumx2, n, logJX, 1., 10., 10., "test")
            stats = f.get_sufficient_statistics()
            self.assertAlmostEqual(stats[0], sumx, delta=1e-4)
            self.assertAlmostEqual(stats[1], sumx2, delta=1e-2)
            self.assertAlmostEqual(stats[2], n, delta=1e-6)
            self.assertAlmostEqual(f.get_LogJX(), logJX, delta=1e-6)

    def test_evaluate(self):
        """Test FStudentT.evaluate"""
        n = 10
        for i in range(10):
            # Test Student-t
            m, s, v = uniform(-10, 10), uniform(1, 10), uniform(1, 60)
            x = np.random.uniform(-100, 100, n)
            jx = np.ones_like(x)
            f = FStudentT(x, jx, m, s, v, "test")
            exp_score = evaluate_studentt(x, m, s, v)
            self.assertAlmostEqual(f.evaluate(), exp_score, delta=1e-6)

            # Test Log-Student-t
            m, s, v = uniform(.1, 10), uniform(1, 10), uniform(1, 60)
            x = np.random.uniform(.1, 100, n)
            jx = 1. / x
            logx = np.log(x)
            logm = np.log(m)
            f = FStudentT(logx, jx, logm, s, v, "test")
            exp_score = evaluate_logstudentt(logx, logm, s, v)
            self.assertAlmostEqual(f.evaluate(), exp_score, delta=1e-6)

    def test_get_density(self):
        """Test FStudentT.get_density"""
        n = 10
        for i in range(10):
            m, s, v = uniform(-10, 10), uniform(1, 10), uniform(1, 60)
            x = np.random.uniform(-100, 100, n)
            jx = np.ones_like(x)
            f = FStudentT(x, jx, m, s, v, "test")
            exp_dens = np.exp(-evaluate_studentt(x, m, s, v))
            self.assertAlmostEqual(f.get_density(), exp_dens, delta=1e-6)

    def test_evaluate_DFM(self):
        """Test FStudentT.evaluate_derivative_FM"""
        n = 10
        for i in range(10):
            m, s, v = uniform(-10, 10), uniform(1, 10), uniform(1, 60)
            x = np.random.uniform(-100, 100, n)
            jx = np.ones_like(x)
            f = FStudentT(x, jx, m, s, v, "test")
            f.update_cached_values()
            exp_derv = evaluate_derivative_fm(x, m, s, v)
            self.assertAlmostEqual(f.evaluate_derivative_FM(), exp_derv,
                                   delta=1e-6)

    def test_evaluate_Dsigma(self):
        """Test FStudentT.evaluate_derivative_sigma"""
        n = 10
        for i in range(10):
            m, s, v = uniform(-10, 10), uniform(1, 10), uniform(1, 60)
            x = np.random.uniform(-100, 100, n)
            jx = np.ones_like(x)
            f = FStudentT(x, jx, m, s, v, "test")
            f.update_cached_values()
            exp_derv = evaluate_derivative_s(x, m, s, v)
            self.assertAlmostEqual(f.evaluate_derivative_sigma(), exp_derv,
                                   delta=1e-6)

    def test_evaluate_Dnu(self):
        """Test FStudentT.evaluate_derivative_nu"""
        n = 10
        for i in range(10):
            m, s, v = uniform(-10, 10), uniform(1, 10), uniform(1, 60)
            x = np.random.uniform(-100, 100, n)
            jx = np.ones_like(x)
            f = FStudentT(x, jx, m, s, v, "test")
            f.update_cached_values()
            exp_derv = evaluate_derivative_v(x, m, s, v)
            self.assertAlmostEqual(f.evaluate_derivative_nu(), exp_derv,
                                   delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
