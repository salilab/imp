import numpy as np
import IMP
import IMP.core
import IMP.isd
import IMP.test


class DummyRestraint(IMP.Restraint):

    """Dummy do-nothing restraint"""

    def __init__(self, m, p, name="DummyRestraint %1%"):
        IMP.Restraint.__init__(self, m, name)
        self.p = p

    def unprotected_evaluate(self, accum):
        return 0.

    def do_get_inputs(self):
        return [self.p]


class Tests(IMP.test.TestCase):

    """Test transformation of Nuisance"""

    def setup_nuisance(self, m):
        p = IMP.Particle(m)
        nuis = IMP.isd.TransformedNuisance.setup_particle(p)
        return nuis

    def setup_lower_upper(self, m):
        nuis = self.setup_nuisance(m)
        nuis.set_lower(-10)
        nuis.set_upper(10)
        return nuis

    def setup_lower(self, m):
        nuis = self.setup_nuisance(m)
        nuis.set_lower(-10)
        return nuis

    def setup_upper(self, m):
        nuis = self.setup_nuisance(m)
        nuis.set_upper(10)
        return nuis

    def test_setup1(self):
        m = IMP.Model()
        nuis = IMP.isd.TransformedNuisance.setup_particle(
            IMP.Particle(m))
        self.assertTrue(
            IMP.isd.Nuisance.get_is_setup(m, nuis))
        self.assertTrue(
            IMP.isd.TransformedNuisance.get_is_setup(m, nuis))
        self.assertAlmostEqual(nuis.get_nuisance(), 1.)

    def test_setup2(self):
        m = IMP.Model()
        nuis = IMP.isd.TransformedNuisance.setup_particle(
            IMP.Particle(m), 10.)
        self.assertTrue(
            IMP.isd.Nuisance.get_is_setup(m, nuis))
        self.assertTrue(
            IMP.isd.TransformedNuisance.get_is_setup(m, nuis))
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)

    def test_get_transformation_type(self):
        m = IMP.Model()
        nuis = self.setup_nuisance(m)
        self.assertEqual(nuis.get_transformation_type(), 0)

        nuis = self.setup_lower(m)
        self.assertEqual(nuis.get_transformation_type(), 1)

        nuis = self.setup_upper(m)
        self.assertEqual(nuis.get_transformation_type(), 2)

        nuis = self.setup_lower_upper(m)
        self.assertEqual(nuis.get_transformation_type(), 3)

    def test_set_remove_lower_preserves_nuisance(self):
        m = IMP.Model()
        nuis = IMP.isd.TransformedNuisance.setup_particle(
            IMP.Particle(m), 10.)

        nuis.set_lower(0)
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)

        nuis.remove_lower()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)

    def test_set_remove_upper_preserves_nuisance(self):
        m = IMP.Model()
        nuis = IMP.isd.TransformedNuisance.setup_particle(
            IMP.Particle(m), 10.)

        nuis.set_upper(20)
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)

        nuis.remove_lower()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 10.)

    def test_set_from_nuisance_sets_nuisance_correctly(self):
        m = IMP.Model()
        nuis = self.setup_nuisance(m)
        nuis.set_from_nuisance(5)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 5, delta=1e-6)

        nuis = self.setup_lower(m)
        nuis.set_from_nuisance(5)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 5, delta=1e-6)

        nuis = self.setup_upper(m)
        nuis.set_from_nuisance(5)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 5, delta=1e-6)

        nuis = self.setup_lower_upper(m)
        nuis.set_from_nuisance(5)
        m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 5, delta=1e-6)

    def test_update_nuisance(self):
        m = IMP.Model()
        nuis = self.setup_lower(m)
        nuis.set_transformed_nuisance(0)
        nuis.update_nuisance()
        self.assertAlmostEqual(nuis.get_nuisance(), -9, delta=1e-6)

    def test_update_nuisance_updates_jacobian(self):
        m = IMP.Model()
        nuis = self.setup_lower(m)
        nuis.set_transformed_nuisance(5)
        nuis.update_nuisance()
        self.assertAlmostEqual(
            nuis.get_jacobian(), np.exp(5), delta=1e-6)

    def test_update_nuisance_updates_score_adjustment(self):
        m = IMP.Model()
        nuis = self.setup_lower(m)
        nuis.set_transformed_nuisance(5)
        nuis.update_nuisance()
        self.assertAlmostEqual(
            nuis.get_score_adjustment(), -5, delta=1e-6)

    def test_update_nuisance_updates_gradient_adjustment(self):
        m = IMP.Model()
        nuis = self.setup_lower(m)
        nuis.set_transformed_nuisance(5)
        nuis.update_nuisance()
        self.assertAlmostEqual(
            nuis.get_gradient_adjustment(), -1, delta=1e-6)

    def test_update_derivative(self):
        m = IMP.Model()
        nuis = self.setup_lower(m)
        nuis.set_transformed_nuisance(5)
        m.update()

        da = IMP.DerivativeAccumulator()
        nuis.add_to_nuisance_derivative(10., da)
        nuis.update_derivative()
        self.assertAlmostEqual(
            nuis.get_transformed_nuisance_derivative(),
            10 * np.exp(5), delta=1e-6)

    def test_lower_bound_transform_is_correct(self):
        m = IMP.Model()
        nuis = self.setup_lower(m)
        nuis.set_lower(-10)
        ys = np.linspace(-10, 10, 21)
        xs = np.exp(ys) - 10
        Js = np.exp(ys)
        for y, x, J in zip(ys, xs, Js):
            nuis.set_transformed_nuisance(y)
            m.update()
            self.assertAlmostEqual(nuis.get_nuisance(), x, delta=1e-6)
            self.assertAlmostEqual(nuis.get_jacobian(), J, delta=1e-6)
            self.assertAlmostEqual(nuis.get_score_adjustment(), -y, delta=1e-6)
            self.assertAlmostEqual(
                nuis.get_gradient_adjustment(), -1, delta=1e-6)

    def test_upper_bound_transform_is_correct(self):
        m = IMP.Model()
        nuis = self.setup_upper(m)
        nuis.set_upper(10)
        ys = np.linspace(-10, 10, 21)
        xs = 10 - np.exp(ys)
        Js = -np.exp(ys)
        for y, x, J in zip(ys, xs, Js):
            nuis.set_transformed_nuisance(y)
            m.update()
            self.assertAlmostEqual(nuis.get_nuisance(), x, delta=1e-6)
            self.assertAlmostEqual(nuis.get_jacobian(), J, delta=1e-6)
            self.assertAlmostEqual(nuis.get_score_adjustment(), -y, delta=1e-6)
            self.assertAlmostEqual(
                nuis.get_gradient_adjustment(), -1, delta=1e-6)

    def test_lower_upper_bound_transform_is_correct(self):
        m = IMP.Model()
        nuis = self.setup_lower_upper(m)
        nuis.set_lower(-10)
        nuis.set_upper(10)
        ys = np.linspace(-10, 10, 21)
        xs = 10 - 20. / (1 + np.exp(ys))
        Js = 20 * np.exp(ys) / (1 + np.exp(ys))**2
        score_adjs = -ys - np.log(20) + 2 * np.log(1 + np.exp(ys))
        grad_adjs = np.tanh(ys / 2.)
        for y, x, J, score_adj, grad_adj in zip(ys, xs, Js,
                                                score_adjs, grad_adjs):
            nuis.set_transformed_nuisance(y)
            m.update()
            self.assertAlmostEqual(nuis.get_nuisance(), x, delta=1e-6)
            self.assertAlmostEqual(nuis.get_jacobian(), J, delta=1e-6)
            self.assertAlmostEqual(
                nuis.get_score_adjustment(), score_adj, delta=1e-6)
            self.assertAlmostEqual(
                nuis.get_gradient_adjustment(), grad_adj, delta=1e-6)

    def test_gradient_adjusted_after_evaluate(self):
        m = IMP.Model()
        nuis = self.setup_lower_upper(m)
        nuis.set_lower(-10)
        nuis.set_upper(10)
        nuis.set_from_nuisance(5)

        r = DummyRestraint(m, nuis.get_particle())
        sf = IMP.core.RestraintsScoringFunction([r])

        self.assertAlmostEqual(
            nuis.get_transformed_nuisance_derivative(), 0, delta=1e-6)
        sf.evaluate(True)
        self.assertAlmostEqual(
            nuis.get_transformed_nuisance_derivative(), 0, delta=1e-6)
        nuis.set_transformed_nuisance_is_optimized(True)
        sf.evaluate(True)
        self.assertAlmostEqual(
            nuis.get_transformed_nuisance_derivative(), .5, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
