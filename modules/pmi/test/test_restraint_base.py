from __future__ import print_function, division
import math
import IMP
import IMP.algebra
import IMP.core
import IMP.isd
import IMP.pmi
import IMP.pmi.restraints
import IMP.test


class DistanceRestraint(IMP.pmi.restraints.RestraintBase):

    def __init__(self, p1, p2, d, k, name=None, label=None, weight=1.):
        m = p1.get_model()
        super(DistanceRestraint, self).__init__(m, name=name, label=label,
                                                weight=weight)
        f = IMP.core.Harmonic(d, k)
        s = IMP.core.DistancePairScore(f)
        r = IMP.core.PairRestraint(self.m, s, (p1, p2))
        self.rs.add_restraint(r)


class Tests(IMP.test.TestCase):

    def test_setup(self):
        m = IMP.Model()
        p1 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p1)
        p2 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p2)

        r = DistanceRestraint(p1, p2, 0., 1.)
        self.assertAlmostEqual(r.evaluate(), 0.0, delta=1e-6)
        output = r.get_output()
        self.assertEqual(output["DistanceRestraint_Score"], str(0.0))
        self.assertEqual(output["_TotalScore"], str(0.0))
        self.assertIsInstance(r.get_restraint_set(), IMP.RestraintSet)

    def test_setup_with_label_weight(self):
        m = IMP.Model()
        p1 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p1)
        p2 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p2)
        IMP.core.XYZ(p2).set_coordinates(IMP.algebra.Vector3D(0, 0, 10))

        r = DistanceRestraint(p1, p2, 0., 1., name="DistanceRestraint2",
                              weight=10., label="Test")
        self.assertAlmostEqual(r.evaluate(), 500, delta=1e-6)
        output = r.get_output()
        self.assertEqual(output["DistanceRestraint2_Score_Test"], str(500.0))
        self.assertEqual(output["_TotalScore"], str(500.0))

        r.set_weight(1.)
        output = r.get_output()
        self.assertEqual(output["DistanceRestraint2_Score_Test"], str(50.0))
        self.assertEqual(output["_TotalScore"], str(50.0))

    def test_set_label_after_init_raises_error(self):
        m = IMP.Model()
        p1 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p1)
        p2 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p2)

        r = DistanceRestraint(p1, p2, 0., 1., label="Test")
        with self.assertRaises(ValueError):
            r.set_label("Test2")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.set_label("Test")
        with self.assertRaises(ValueError):
            r.set_label("Test2")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.get_output()
        with self.assertRaises(ValueError):
            r.set_label("Test")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.evaluate()
        with self.assertRaises(ValueError):
            r.set_label("Test")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.add_to_model()
        with self.assertRaises(ValueError):
            r.set_label("Test")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.get_restraint_set()
        with self.assertRaises(ValueError):
            r.set_label("Test")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.get_restraint_for_rmf()
        with self.assertRaises(ValueError):
            r.set_label("Test")

        r = DistanceRestraint(p1, p2, 0., 1.)
        r.get_particles_to_sample()
        with self.assertRaises(ValueError):
            r.set_label("Test")

    def test_setup_with_nuisance(self):

        class GaussianRestraint(IMP.pmi.restraints._RestraintNuisanceMixin,
                                IMP.pmi.restraints.RestraintBase):

            def __init__(self, p, mean_val, label=None, weight=1.):
                m = p.get_model()
                super(GaussianRestraint, self).__init__(m, label=label,
                                                        weight=weight)

                self.mu = self._create_nuisance(mean_val, None, None, None,
                                                "Mu", is_sampled=False)
                self.sigma = self._create_nuisance(1., 0., None, .1, "Sigma",
                                                   is_sampled=True)

                r = IMP.isd.GaussianRestraint(p, self.mu, self.sigma)
                self.rs.add_restraint(r)

                self.rs_jeffreys = self._create_restraint_set(
                    "Sigma_JeffreysPrior")
                r = IMP.isd.JeffreysRestraint(self.m, self.sigma)
                self.rs_jeffreys.add_restraint(r)

        def calculate_gaussian(x, mu, sigma):
            return -math.log(1. / math.sqrt(2 * math.pi * sigma**2) *
                             math.exp(-(x - mu)**2 / 2. / sigma**2))

        m = IMP.Model()
        p = IMP.Particle(m)
        pnuis = IMP.isd.Nuisance.setup_particle(p)
        pnuis.set_nuisance(10.)

        r = GaussianRestraint(p, 10., label="Test")
        self.assertAlmostEqual(r.evaluate(), calculate_gaussian(10., 10., 1.),
                               delta=1e-6)

        r = GaussianRestraint(p, 11., label="Test")
        self.assertAlmostEqual(r.evaluate(), calculate_gaussian(11., 10., 1.),
                               delta=1e-6)

        self.assertEqual(len(r.restraint_sets), 2)
        self.assertEqual(len(r.get_particles_to_sample()), 1)
        self.assertIs(list(r.get_particles_to_sample().values())[0][0][0],
                      r.sigma)
        self.assertEqual(len(r.get_output()), 5)

        output = r.get_output()
        self.assertAlmostEqual(
            float(output["GaussianRestraint_Sigma_JeffreysPrior_Score_Test"]),
            0., delta=1e-6)

        self.assertFalse(r.mu.get_nuisance_is_optimized())
        self.assertTrue(r.sigma.get_nuisance_is_optimized())
        r.sigma.set_nuisance(10.)
        output = r.get_output()
        self.assertAlmostEqual(
            float(output["GaussianRestraint_Sigma_JeffreysPrior_Score_Test"]),
            -math.log(.1), delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
