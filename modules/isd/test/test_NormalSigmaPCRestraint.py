#!/usr/bin/env python
from __future__ import division
import math
import random

import IMP
from IMP.isd import NormalSigmaPCRestraint, Scale
import IMP.test


def get_score(s, su, a):
    return -math.log(-math.log(a) / su) - math.log(a) * s / su


def get_derivative_score_wrt_s(s, su, a):
    return -math.log(a) / su


def create_scale(m, v=None):
    p = IMP.Particle(m)
    if v is None:
        v = random.uniform(0, 50)
    return Scale.setup_particle(p, v)


class Tests(IMP.test.TestCase):

    """Tests for NormalSigmaPCRestraint."""

    def test_create_restraint1(self):
        m = IMP.Model()
        s = create_scale(m)
        r = NormalSigmaPCRestraint(s, 10, .01)
        self.assertEqual(s, r.get_sigma())
        self.assertAlmostEqual(r.get_sigma_upper(), 10)
        self.assertAlmostEqual(r.get_alpha(), .01)
        r.set_was_used(True)

    def test_create_restraint2(self):
        m = IMP.Model()
        s = create_scale(m)
        r = NormalSigmaPCRestraint(m, s.get_particle_index(), 20, .001)
        self.assertEqual(s, r.get_sigma())
        self.assertAlmostEqual(r.get_sigma_upper(), 20)
        self.assertAlmostEqual(r.get_alpha(), .001)
        r.set_was_used(True)

    def test_evaluate_score_many(self):
        """Test NormalSigmaPCRestraint.unprotected_evaluate"""
        m = IMP.Model()

        n = 20
        for i in range(n):
            sval = random.uniform(0, 10)
            su = random.uniform(5, 20)
            s = create_scale(m, sval)
            a = random.uniform(0, .5)
            r = NormalSigmaPCRestraint(s, su, a)
            exp_score = get_score(sval, su, a)
            self.assertAlmostEqual(r.unprotected_evaluate(None), exp_score,
                                   delta=1e-6)
            r.set_was_used(True)

    def test_evaluate_score_sigma_0(self):
        """Test NormalSigmaPCRestraint.unprotected_evaluate for sigma=0"""
        m = IMP.Model()

        n = 20
        for i in range(n):
            su = random.uniform(5, 20)
            s = create_scale(m, 0)
            a = random.uniform(0, .5)
            r = NormalSigmaPCRestraint(s, su, a)
            exp_score = math.log(su / (-math.log(a)))
            self.assertAlmostEqual(r.unprotected_evaluate(None), exp_score,
                                   delta=1e-6)
            r.set_was_used(True)

    def test_get_probability_many(self):
        """Test NormalSigmaPCRestraint.get_probability"""
        m = IMP.Model()

        n = 20
        for i in range(n):
            sval = random.uniform(0, 10)
            su = random.uniform(5, 20)
            s = create_scale(m, sval)
            a = random.uniform(0, .5)
            r = NormalSigmaPCRestraint(s, su, a)
            exp_dens = math.exp(-get_score(sval, su, a))
            self.assertAlmostEqual(r.get_probability(), exp_dens, delta=1e-6)
            r.set_was_used(True)

    def test_evaluate_derivative_wrt_sigma(self):
        """Test NormalSigmaPCRestraint.evaluate_derivative_Dxi"""
        m = IMP.Model()

        n = 10
        for i in range(n):
            su = random.uniform(5, 20)
            s = create_scale(m)
            a = random.uniform(0, .5)
            r = NormalSigmaPCRestraint(s, su, a)
            exp_deriv = -math.log(a) / su
            da = IMP.DerivativeAccumulator()
            r.unprotected_evaluate(da)
            self.assertAlmostEqual(s.get_scale_derivative(), exp_deriv,
                                   delta=1e-6)
            r.set_was_used(True)

    def test_get_inputs(self):
        """Test NormalSigmaPCRestraint.get_inputs"""
        m = IMP.Model()
        s = create_scale(m)
        r = NormalSigmaPCRestraint(s, 50, .01)
        self.assertEqual(len(r.get_inputs()), 1)
        self.assertEqual(r.get_inputs()[0], s.get_particle())
        r.set_was_used(True)


if __name__ == '__main__':
    IMP.test.main()
