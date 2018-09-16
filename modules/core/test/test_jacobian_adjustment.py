from __future__ import print_function
import numpy as np
import IMP
import IMP.core
import IMP.test


class DummyRestraint(IMP.Restraint):

    """Dummy do-nothing restraint"""

    def __init__(self, m, p, k, name="DummyRestraint %1%"):
        IMP.Restraint.__init__(self, m, name)
        self.p = p
        self.k = k

    def unprotected_evaluate(self, accum):
        if accum:
            self.p.add_to_derivative(self.k, 0., accum)
        return 0.

    def do_get_inputs(self):
        return [self.p]


class Tests(IMP.test.TestCase):

    """Tests for classes for Jacobian adjustments to score and its gradient."""

    @staticmethod
    def get_random_multivariate_jacobian_factors(N):
        J = np.random.normal(size=(N, N))
        score_adj = np.random.normal()
        grad_adj = np.random.normal(size=N)
        return J, score_adj, grad_adj

    def test_create_univariate_jacobian(self):
        vs = np.random.normal(size=3)
        j = IMP.core.UnivariateJacobian(*vs)
        self.assertAlmostEqual(j.get_jacobian(), vs[0], delta=1e-6)
        self.assertAlmostEqual(j.get_score_adjustment(), vs[1], delta=1e-6)
        self.assertAlmostEqual(j.get_gradient_adjustment(), vs[2], delta=1e-6)

    def test_get_set_univariate_jacobian(self):
        j = IMP.core.UnivariateJacobian(1, 0, 0)
        vs = np.random.normal(size=3)
        j.set_jacobian(vs[0])
        self.assertAlmostEqual(j.get_jacobian(), vs[0], delta=1e-6)
        j.set_score_adjustment(vs[1])
        self.assertAlmostEqual(j.get_score_adjustment(), vs[1], delta=1e-6)
        j.set_gradient_adjustment(vs[2])
        self.assertAlmostEqual(j.get_gradient_adjustment(), vs[2], delta=1e-6)

    def test_create_multivariate_jacobian(self):
        N = 2
        J, score_adj, grad_adj = self.get_random_multivariate_jacobian_factors(
            N
        )
        j = IMP.core.MultivariateJacobian(J, score_adj, grad_adj)

        np.testing.assert_allclose(j.get_jacobian(), J, atol=1e-6)
        self.assertAlmostEqual(j.get_score_adjustment(), score_adj, delta=1e-6)
        np.testing.assert_allclose(
            j.get_gradient_adjustment(), grad_adj, atol=1e-6
        )

    def test_multivariate_jacobian_create_with_incompatible_shapes_raises_error(
        self
    ):
        N = 2
        J, score_adj, _ = self.get_random_multivariate_jacobian_factors(N)
        grad_adj = np.random.normal(size=N + 1)
        self.assertRaisesUsageException(
            IMP.core.MultivariateJacobian, J, score_adj, grad_adj
        )

        J = np.random.normal(size=(N, N + 1))
        grad_adj = np.random.normal(size=N)
        self.assertRaisesUsageException(
            IMP.core.MultivariateJacobian, J, score_adj, grad_adj
        )

        J = np.random.normal(size=(N, N))
        IMP.core.MultivariateJacobian(J, score_adj, grad_adj)

    def test_get_set_multivariate_jacobian(self):
        N = 2
        J = np.random.normal(size=(N, N))
        score_adj = np.random.normal()
        grad_adj = np.random.normal(size=N)
        j = IMP.core.MultivariateJacobian(
            *self.get_random_multivariate_jacobian_factors(N)
        )

        J, score_adj, grad_adj = self.get_random_multivariate_jacobian_factors(
            N
        )
        j.set_jacobian(J)
        j.set_score_adjustment(score_adj)
        j.set_gradient_adjustment(grad_adj)

        np.testing.assert_allclose(j.get_jacobian(), J, atol=1e-6)
        self.assertAlmostEqual(j.get_score_adjustment(), score_adj, delta=1e-6)
        np.testing.assert_allclose(
            j.get_gradient_adjustment(), grad_adj, atol=1e-6
        )

    def test_get_jacobian_adjuster(self):
        m = IMP.Model()
        ja1 = IMP.core.get_jacobian_adjuster(m)
        ja2 = IMP.core.get_jacobian_adjuster(m)
        self.assertEqual(ja1, ja2)

    def test_get_set_temperature(self):
        m = IMP.Model()
        ja = IMP.core.get_jacobian_adjuster(m)
        self.assertAlmostEqual(ja.get_temperature(), 1)
        ja.set_temperature(10)
        self.assertAlmostEqual(ja.get_temperature(), 10)

    def test_univariate_jacobian_roundtrip(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        k = IMP.FloatKey("dummy")
        p.add_attribute(k, 10.)
        vs = np.random.normal(size=3)
        j1 = IMP.core.UnivariateJacobian(*vs)
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(k, p.get_index(), j1)
        j2 = ja.get_jacobian(k, p)
        self.assertEqual(j1, j2)

    def test_set_univariate_jacobian_without_attribute_raises_usage_error(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        k = IMP.FloatKey("dummy")
        vs = np.random.normal(size=3)
        j = IMP.core.UnivariateJacobian(*vs)
        ja = IMP.core.get_jacobian_adjuster(m)
        self.assertRaisesUsageException(ja.set_jacobian, k, p.get_index(), j)

    def test_univariate_get_inputs(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        k = IMP.FloatKey("dummy")
        p.add_attribute(k, 10.)
        j = IMP.core.UnivariateJacobian(1, 0, 0)
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(k, p.get_index(), j)
        self.assertListEqual(ja.get_inputs(), [p])

    def test_univariate_get_outputs(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        k = IMP.FloatKey("dummy")
        p.add_attribute(k, 10.)
        j = IMP.core.UnivariateJacobian(1, 0, 0)
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(k, p.get_index(), j)
        self.assertListEqual(ja.get_outputs(), [p])

    def test_univariate_get_score_adjustment(self):
        m = IMP.Model()
        ps = [IMP.Particle(m) for i in range(2)]
        vs = [np.random.normal(size=3) for _ in ps]
        ja = IMP.core.get_jacobian_adjuster(m)
        for p, v in zip(ps, vs):
            k = IMP.FloatKey("dummy")
            p.add_attribute(k, 10.)
            j = IMP.core.UnivariateJacobian(*v)
            ja.set_jacobian(k, p.get_index(), j)

        self.assertAlmostEqual(ja.get_score_adjustment(), 0, delta=1e-6)

        ps[0].set_is_optimized(k, True)
        self.assertAlmostEqual(ja.get_score_adjustment(), vs[0][1], delta=1e-6)

    def test_univariate_apply_gradient_adjustment(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        k = IMP.FloatKey("dummy")
        p.add_attribute(k, 10.)
        vs = np.random.normal(size=3)
        j = IMP.core.UnivariateJacobian(*vs)
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(k, p.get_index(), j)

        self.assertAlmostEqual(p.get_derivative(k), 0., delta=1e-6)

        ja.apply_gradient_adjustment()
        self.assertAlmostEqual(p.get_derivative(k), 0., delta=1e-6)

        p.set_is_optimized(k, True)
        ja.apply_gradient_adjustment()
        self.assertAlmostEqual(p.get_derivative(k), vs[2], delta=1e-6)

    def test_univariate_score_state_applies_gradient_adjustment(self):
        m = IMP.Model()
        ps = [IMP.Particle(m) for i in range(2)]
        vs = [np.random.normal(size=3) for _ in ps]
        ja = IMP.core.get_jacobian_adjuster(m)
        rs = []
        for p, v in zip(ps, vs):
            k = IMP.FloatKey("dummy")
            p.add_attribute(k, 10.)
            j = IMP.core.UnivariateJacobian(*v)
            ja.set_jacobian(k, p.get_index(), j)
            r = DummyRestraint(m, p, k)
            rs.append(r)

        sf = IMP.core.RestraintsScoringFunction(rs)

        self.assertAlmostEqual(ps[0].get_derivative(k), 0, delta=1e-6)
        self.assertAlmostEqual(ps[1].get_derivative(k), 0, delta=1e-6)

        sf.evaluate(True)
        self.assertAlmostEqual(ps[0].get_derivative(k), 0, delta=1e-6)
        self.assertAlmostEqual(ps[1].get_derivative(k), 0, delta=1e-6)

        ps[0].set_is_optimized(k, True)
        sf.evaluate(True)
        self.assertAlmostEqual(ps[0].get_derivative(k), vs[0][2], delta=1e-6)
        self.assertAlmostEqual(ps[1].get_derivative(k), 0, delta=1e-6)

    def test_multivariate_jacobian_roundtrip(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        N = 2
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        for k in ks:
            p.add_attribute(k, 1.)
        j1 = IMP.core.MultivariateJacobian(
            *self.get_random_multivariate_jacobian_factors(N)
        )
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(ks, p.get_index(), j1)
        j2 = ja.get_jacobian(ks, p)
        self.assertEqual(j1, j2)

    def test_set_multivariate_jacobian_without_attribute_raises_usage_error(
        self
    ):
        m = IMP.Model()
        p = IMP.Particle(m)
        N = 2
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        j = IMP.core.MultivariateJacobian(
            *self.get_random_multivariate_jacobian_factors(N)
        )
        ja = IMP.core.get_jacobian_adjuster(m)
        self.assertRaisesUsageException(ja.set_jacobian, ks, p.get_index(), j)

    def test_multivariate_get_inputs(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        N = 2
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        for k in ks:
            p.add_attribute(k, 1.)
        j = IMP.core.MultivariateJacobian(
            *self.get_random_multivariate_jacobian_factors(N)
        )
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(ks, p.get_index(), j)
        self.assertListEqual(ja.get_inputs(), [p])

    def test_multivariate_get_outputs(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        N = 2
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        for k in ks:
            p.add_attribute(k, 1.)
        j = IMP.core.MultivariateJacobian(
            *self.get_random_multivariate_jacobian_factors(N)
        )
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_jacobian(ks, p.get_index(), j)
        self.assertListEqual(ja.get_outputs(), [p])

    def test_multivariate_get_score_adjustment(self):
        m = IMP.Model()
        ps = [IMP.Particle(m) for i in range(2)]
        N = 2
        ja = IMP.core.get_jacobian_adjuster(m)
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        score_adj = []
        for p in ps:
            for k in ks:
                p.add_attribute(k, 1.)
                p.set_is_optimized(k, False)
            j = IMP.core.MultivariateJacobian(
                *self.get_random_multivariate_jacobian_factors(N)
            )
            ja.set_jacobian(ks, p.get_index(), j)
            score_adj.append(j.get_score_adjustment())

        self.assertAlmostEqual(ja.get_score_adjustment(), 0, delta=1e-6)

        for k in ks:
            ps[0].set_is_optimized(k, True)
        self.assertAlmostEqual(
            ja.get_score_adjustment(), score_adj[0], delta=1e-6
        )

    def test_multivariate_apply_gradient_adjustment(self):
        m = IMP.Model()
        ps = [IMP.Particle(m) for i in range(2)]
        N = 2
        ja = IMP.core.get_jacobian_adjuster(m)
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        grad_adj = []
        for p in ps:
            for k in ks:
                p.add_attribute(k, 1.)
                p.set_is_optimized(k, False)
            j = IMP.core.MultivariateJacobian(
                *self.get_random_multivariate_jacobian_factors(N)
            )
            ja.set_jacobian(ks, p.get_index(), j)
            grad_adj.append(j.get_gradient_adjustment())

        for k in ks:
            self.assertAlmostEqual(ps[0].get_derivative(k), 0., delta=1e-6)
            self.assertAlmostEqual(ps[1].get_derivative(k), 0., delta=1e-6)

        ja.apply_gradient_adjustment()
        for k in ks:
            self.assertAlmostEqual(ps[0].get_derivative(k), 0., delta=1e-6)
            self.assertAlmostEqual(ps[1].get_derivative(k), 0., delta=1e-6)

        for k in ks:
            ps[0].set_is_optimized(k, True)
        ja.apply_gradient_adjustment()
        for i, k in enumerate(ks):
            self.assertAlmostEqual(
                ps[0].get_derivative(k), grad_adj[0][i], delta=1e-6
            )
            self.assertAlmostEqual(ps[1].get_derivative(k), 0., delta=1e-6)

    def test_multivariate_score_state_applies_gradient_adjustment(self):
        m = IMP.Model()
        ps = [IMP.Particle(m) for i in range(2)]
        N = 2
        ja = IMP.core.get_jacobian_adjuster(m)
        ks = [IMP.FloatKey("dummy" + str(i)) for i in range(N)]
        grad_adj = []
        rs = []
        for p in ps:
            for k in ks:
                p.add_attribute(k, 1.)
                p.set_is_optimized(k, False)
                r = DummyRestraint(m, p, k)
                rs.append(r)
            j = IMP.core.MultivariateJacobian(
                *self.get_random_multivariate_jacobian_factors(N)
            )
            ja.set_jacobian(ks, p.get_index(), j)
            grad_adj.append(j.get_gradient_adjustment())

        sf = IMP.core.RestraintsScoringFunction(rs)

        for k in ks:
            self.assertAlmostEqual(ps[0].get_derivative(k), 0, delta=1e-6)
            self.assertAlmostEqual(ps[1].get_derivative(k), 0, delta=1e-6)

        sf.evaluate(True)
        for k in ks:
            self.assertAlmostEqual(ps[0].get_derivative(k), 0, delta=1e-6)
            self.assertAlmostEqual(ps[1].get_derivative(k), 0, delta=1e-6)

        for k in ks:
            ps[0].set_is_optimized(k, True)
        sf.evaluate(True)
        for i, k in enumerate(ks):
            self.assertAlmostEqual(
                ps[0].get_derivative(k), grad_adj[0][i], delta=1e-6
            )
            self.assertAlmostEqual(ps[1].get_derivative(k), 0, delta=1e-6)

    def test_get_tempered_score_adjustment(self):
        m = IMP.Model()
        ps = [IMP.Particle(m) for i in range(2)]
        vs = [np.random.normal(size=3) for _ in ps]
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_temperature(100)
        for p, v in zip(ps, vs):
            k = IMP.FloatKey("dummy")
            p.add_attribute(k, 10.)
            j = IMP.core.UnivariateJacobian(*v)
            ja.set_jacobian(k, p.get_index(), j)

        self.assertAlmostEqual(ja.get_score_adjustment(), 0, delta=1e-6)

        ps[0].set_is_optimized(k, True)
        self.assertAlmostEqual(
            ja.get_score_adjustment(), 100 * vs[0][1], delta=1e-6)

    def test_apply_tempered_gradient_adjustment(self):
        m = IMP.Model()
        p = IMP.Particle(m)
        k = IMP.FloatKey("dummy")
        p.add_attribute(k, 10.)
        vs = np.random.normal(size=3)
        j = IMP.core.UnivariateJacobian(*vs)
        ja = IMP.core.get_jacobian_adjuster(m)
        ja.set_temperature(100)
        ja.set_jacobian(k, p.get_index(), j)

        self.assertAlmostEqual(p.get_derivative(k), 0., delta=1e-6)

        ja.apply_gradient_adjustment()
        self.assertAlmostEqual(p.get_derivative(k), 0., delta=1e-6)

        p.set_is_optimized(k, True)
        ja.apply_gradient_adjustment()
        self.assertAlmostEqual(p.get_derivative(k), 100 * vs[2], delta=1e-6)


if __name__ == "__main__":
    IMP.test.main()
