#!/usr/bin/env python
from __future__ import division
from random import uniform

import numpy as np
try:
    import scipy.special
except ImportError:
    scipy = None
import IMP
from IMP.isd import FStudentT
import IMP.test


def evaluate_fstudentt(fxs, jxs, fm, s, v):
    n = float(fxs.size)
    sumfx = np.sum(fxs)
    sumfx2 = np.sum(fxs**2)
    logjx = np.sum(np.log(jxs))
    t2 = (sumfx2 - 2 * fm * sumfx + n * fm**2) / s**2
    return (scipy.special.gammaln(.5 * v)
            - scipy.special.gammaln(.5 * (n + v))
            + .5 * n * np.log(np.pi * v) + n * np.log(s)
            + .5 * (n + v) * np.log(1 + t2 / v) - logjx)


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
    return .5 * (-1 + scipy.special.digamma(.5 * v)
                 - scipy.special.digamma(.5 * (n + v))
                 + np.log(1 + t2 / v) + (n + v) / (v + t2))


def get_random_attributes():
    attributes = np.random.uniform(1., 100, 6)
    attributes = list(attributes)
    attributes = attributes[:2] + [np.random.randint(1, 100)] + attributes[2:]
    return attributes


class Tests(IMP.test.TestCase):

    """Tests for FStudentT."""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)
        if scipy is None:
            self.skipTest("No scipy.special module")

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

    def test_setters_getters(self):
        f = FStudentT(*get_random_attributes())
        for i in range(10):
            vs = get_random_attributes()
            f.set_sumFX(vs[0])
            f.set_sumFX2(vs[1])
            f.set_N(vs[2])
            f.set_LogJX(vs[3])
            f.set_FM(vs[4])
            f.set_sigma(vs[5])
            f.set_nu(vs[6])
            self.assertAlmostEqual(f.get_sumFX(), vs[0], delta=1e-6)
            self.assertAlmostEqual(f.get_sumFX2(), vs[1], delta=1e-6)
            self.assertAlmostEqual(f.get_N(), vs[2], delta=1e-6)
            self.assertAlmostEqual(f.get_LogJX(), vs[3], delta=1e-6)
            self.assertAlmostEqual(f.get_FM(), vs[4], delta=1e-6)
            self.assertAlmostEqual(f.get_sigma(), vs[5], delta=1e-6)
            self.assertAlmostEqual(f.get_nu(), vs[6], delta=1e-6)

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
