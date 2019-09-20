#!/usr/bin/env python

# imp general
import IMP
import IMP.core

# our project
from IMP.isd import Weight

# unit testing framework
import IMP.test

import numpy as np


class TestWeightParam(IMP.test.TestCase):

    """tests weight setup"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()

    def test_setup_empty_add_weight(self):
        "Test setup weight as empty and add weights"
        w = Weight.setup_particle(IMP.Particle(self.m))
        for n in range(1, 20):
            w.add_weight()
            self.assertEqual(w.get_number_of_weights(), n)
            self.assertSequenceAlmostEqual(
                list(w.get_weights()), [1] + [0] * (n - 1)
            )

    def test_setup_number_of_weights(self):
        "Test setup weight with number of weights"
        for n in range(1, 20):
            p = IMP.Particle(self.m)
            w = Weight.setup_particle(p, n)
            self.assertTrue(Weight.get_is_setup(p))
            self.assertEqual(w.get_number_of_weights(), n)
            for k in range(n):
                self.assertAlmostEqual(w.get_weight(k), 1.0 / n, delta=1e-6)

    def test_setup_weights(self):
        "Test setup weight with initial values"
        for n in range(1, 20):
            p = IMP.Particle(self.m)
            ws = np.random.uniform(size=n)
            w = Weight.setup_particle(p, ws)
            self.assertTrue(Weight.get_is_setup(p))
            self.assertEqual(w.get_number_of_weights(), n)
            self.assertSequenceAlmostEqual(
                w.get_weights(),
                IMP.algebra.get_projected(
                    w.get_unit_simplex(), IMP.algebra.VectorKD(ws)
                ),
            )

    def test_set_weights(self):
        for n in range(1, 20):
            p = IMP.Particle(self.m)
            w = Weight.setup_particle(p, n)
            ws = np.random.uniform(size=n)
            w.set_weights(ws)
            self.assertSequenceAlmostEqual(
                w.get_weights(),
                IMP.algebra.get_projected(
                    w.get_unit_simplex(), IMP.algebra.VectorKD(ws)
                ),
            )

    def test_set_weights_lazy(self):
        for n in range(1, 20):
            p = IMP.Particle(self.m)
            w = Weight.setup_particle(p, n)
            ws = np.random.uniform(size=n)
            w.set_weights_lazy(ws)
            for k in range(n):
                self.assertAlmostEqual(w.get_weight(k), ws[k], delta=1e-6)

            self.m.update()
            self.assertSequenceAlmostEqual(
                w.get_weights(),
                IMP.algebra.get_projected(
                    w.get_unit_simplex(), IMP.algebra.VectorKD(ws)
                ),
            )

    def test_set_weight_lazy(self):
        for n in range(1, 20):
            p = IMP.Particle(self.m)
            w = Weight.setup_particle(p, n)
            ws = np.random.uniform(size=n)
            for k in range(n):
                w.set_weight_lazy(k, ws[k])
                self.assertAlmostEqual(w.get_weight(k), ws[k], delta=1e-6)

            self.m.update()
            self.assertSequenceAlmostEqual(
                w.get_weights(),
                IMP.algebra.get_projected(
                    w.get_unit_simplex(), IMP.algebra.VectorKD(ws)
                ),
            )

    def test_set_weights_zero(self):
        p = IMP.Particle(self.m)
        n = 5
        w = Weight.setup_particle(p, n)
        ws = np.zeros(n)
        w.set_weights(ws)
        for k in range(n):
            self.assertAlmostEqual(w.get_weight(k), 1.0 / n, delta=1e-6)

    def test_add_to_weight_derivative(self):
        for n in range(1, 20):
            w = Weight.setup_particle(IMP.Particle(self.m), n)
            ws = np.random.uniform(size=n)
            ws /= np.sum(ws)
            w.set_weights(ws)

            for k in range(0, n):
                dwk = np.random.normal()
                w.add_to_weight_derivative(k, dwk, IMP.DerivativeAccumulator())
                self.assertAlmostEqual(
                    w.get_weight_derivative(k), dwk, delta=1e-6
                )

    def test_add_to_weights_derivatives(self):
        for n in range(1, 20):
            w = Weight.setup_particle(IMP.Particle(self.m), n)
            ws = np.random.uniform(size=n)
            ws /= np.sum(ws)
            w.set_weights(ws)

            dw = np.random.normal(size=n)
            w.add_to_weights_derivatives(dw, IMP.DerivativeAccumulator())
            dw2 = w.get_weights_derivatives()
            dw2 = [dw2[i] for i in range(n)]
            self.assertSequenceAlmostEqual(list(dw), dw2, delta=1e-6)

    def test_add_weight(self):
        "Test add_weight"
        w = Weight.setup_particle(IMP.Particle(self.m), 1)
        for n in range(2, 20):
            ws = list(w.get_weights())
            wi = np.random.uniform()
            w.add_weight(wi)
            ws.append(wi)
            self.assertEqual(w.get_number_of_weights(), n)
            ws = list(IMP.algebra.get_projected(w.get_unit_simplex(), ws))
            self.assertSequenceAlmostEqual(
                list(w.get_weights()), ws
            )

    def test_set_optimized(self):
        "Test weights_optimized"
        for n in range(1, 20):
            w = Weight.setup_particle(IMP.Particle(self.m), n)
            self.assertFalse(w.get_weights_are_optimized())
            w.set_weights_are_optimized(True)
            for k in range(n):
                b = w.get_is_optimized(w.get_weight_key(k))
                self.assertEqual(b, True)
            self.assertTrue(w.get_weights_are_optimized())
            w.set_weights_are_optimized(False)
            for k in range(n):
                b = w.get_is_optimized(w.get_weight_key(k))
                self.assertEqual(b, False)
            self.assertFalse(w.get_weights_are_optimized())

    def test_get_weight_keys(self):
        for n in range(1, 20):
            w = Weight.setup_particle(IMP.Particle(self.m), n)
            wks = [w.get_weight_key(i) for i in range(n)]
            self.assertSequenceEqual(wks, list(w.get_weight_keys()))

    def test_get_unit_simplex(self):
        for n in range(1, 20):
            p = IMP.Particle(self.m)
            ws = np.random.uniform(size=n)
            w = Weight.setup_particle(p, ws)
            s = w.get_unit_simplex()
            self.assertIsInstance(s, IMP.algebra.UnitSimplexKD)
            self.assertEqual(s.get_dimension(), n)


if __name__ == "__main__":
    IMP.test.main()
