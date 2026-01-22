import io
import random
import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.container
try:
    import jax
except ImportError:
    jax = None


def make_test_pair_score(min_distance=9.0, max_distance=10.0,
                         with_container=False,
                         with_charge0=True,
                         with_charge1=True):
    m = IMP.Model()
    p0 = m.add_particle("p0")
    if with_charge0:
        d0 = IMP.atom.Charged.setup_particle(
            m, p0, IMP.algebra.Vector3D(0, 0, 0), 0.0)
    else:
        d0 = IMP.core.XYZ.setup_particle(
            m, p0, IMP.algebra.Vector3D(0, 0, 0))
    p1 = m.add_particle("p1")
    if with_charge1:
        d1 = IMP.atom.Charged.setup_particle(
            m, p1, IMP.algebra.Vector3D(0, 0, 0), 0.0)
    else:
        d1 = IMP.core.XYZ.setup_particle(
            m, p1, IMP.algebra.Vector3D(0, 0, 0))
    sm = IMP.atom.ForceSwitch(min_distance, max_distance)
    c = IMP.atom.CoulombPairScore(sm)
    if with_container:
        lpc = IMP.container.ListPairContainer(m, [(p0, p1)])
        r = IMP.container.PairsRestraint(c, lpc)
    else:
        r = IMP.core.PairRestraint(m, c, (p0, p1))
    sf = IMP.core.RestraintsScoringFunction([r])
    return m, sf, d0, d1, c


def place_xyzs(xyz0, xyz1, box, dist):
    """Place two XYZ particles randomly a given distance apart"""
    v = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(-box, box))
    delta = IMP.algebra.get_random_vector_on(
        IMP.algebra.get_unit_sphere_3d()) * dist
    xyz0.set_coordinates(v)
    xyz1.set_coordinates(v + delta)


class Tests(IMP.test.TestCase):

    """Test the CoulombPairScore"""

    def test_standard_object_methods(self):
        """Check CoulombPairScore standard object methods"""
        sm = IMP.atom.ForceSwitch(9.0, 10.0)
        c = IMP.atom.CoulombPairScore(sm)
        self.check_standard_object_methods(c)

    def test_get_set(self):
        """Check CoulombPairScore get/set methods"""
        sm = IMP.atom.ForceSwitch(9.0, 10.0)
        c = IMP.atom.CoulombPairScore(sm)
        self.assertEqual(c.get_relative_dielectric(), 1.0)
        c.set_relative_dielectric(5.0)
        self.assertEqual(c.get_relative_dielectric(), 5.0)

    def test_value(self):
        """Check score value of CoulombPairScore"""
        m, sf, d0, d1, c = make_test_pair_score()

        box = IMP.algebra.Vector3D(10.0, 20.0, 30.0)
        for q0 in (2.0, -1.0, 0.0):
            d0.set_charge(q0)
            for q1 in (2.0, -1.0, 0.0):
                d1.set_charge(q1)
                for eps in (1.0, 5.0):
                    c.set_relative_dielectric(eps)
                    for dist in (0.5, 1.0, 2.0, 3.0):
                        place_xyzs(d0, d1, box, dist)
                        score = sf.evaluate(False)
                        # Conversion factor to get score in kcal/mol
                        expected = 331.8469014486 * q0 * q1 / dist / eps
                        self.assertAlmostEqual(score, expected, delta=1e-2)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Check JAX implementation of CoulombPairScore"""
        m, sf, d0, d1, c = make_test_pair_score()

        box = IMP.algebra.Vector3D(10.0, 20.0, 30.0)
        for eps in (1.0, 5.0):
            c.set_relative_dielectric(eps)
            ji = sf._get_jax()
            j = jax.jit(ji.score_func)
            X = ji.get_jax_model()
            for q0 in (2.0, -1.0, 0.0):
                d0.set_charge(q0)
                for q1 in (2.0, -1.0, 0.0):
                    d1.set_charge(q1)
                    for dist in (0.5, 1.0, 2.0, 3.0):
                        place_xyzs(d0, d1, box, dist)
                        score = sf.evaluate(False)
                        self.assertAlmostEqual(score, j(X), delta=1e-2)

    def test_derivatives(self):
        """Check derivatives of CoulombPairScore"""
        m, sf, d0, d1, c = make_test_pair_score(4.0, 6.0)
        d0.set_charge(1.0)
        d1.set_charge(1.0)
        # Place one particle at the origin and the other at a random position
        # between 1 and 6 angstroms away (not too close since the derivatives
        # are too large there)
        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        d1.set_coordinates(IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
                           * (random.random() * 5.0 + 1.0))
        self.assertXYZDerivativesInTolerance(sf, d0, 2.0, 3.0)
        self.assertXYZDerivativesInTolerance(sf, d1, 2.0, 3.0)

    def test_smoothing(self):
        """Check smoothing of CoulombPairScore"""
        m, sf, d0, d1, c = make_test_pair_score()
        smm, smsf, smd0, smd1, smc = make_test_pair_score(min_distance=4.0,
                                                          max_distance=5.0)
        box = IMP.algebra.Vector3D(10.0, 20.0, 30.0)

        def place_all(dist):
            place_xyzs(d0, d1, box, dist)
            smd0.set_coordinates(d0.get_coordinates())
            smd1.set_coordinates(d1.get_coordinates())

        for d in (d0, d1, smd0, smd1):
            d.set_charge(1.0)

        # For dist <= min_distance, scores should be identical
        for dist in (3.0, 3.5, 4.0):
            place_all(dist)
            self.assertAlmostEqual(sf.evaluate(False), smsf.evaluate(False),
                                   delta=1e-6)

        # For dist > max_distance, smoothed score should be zero
        place_all(5.5)
        self.assertEqual(smsf.evaluate(False), 0.0)
        self.assertNotEqual(sf.evaluate(False), 0.0)

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_no_model_charge_attribute(self):
        """Test handling of Model with no charge attribute"""
        for container in (False, True):
            m, sf, d0, d1, c = make_test_pair_score(
                with_container=container, with_charge0=False,
                with_charge1=False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              sf.evaluate, False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              c.check_indexes, m, [d0, d1])

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_charge_out_of_range(self):
        """Test handling of Model with charge attribute out of range"""
        # We only add charge to the first particle, so the dimension of
        # the charge array will be 1, not the full number of particles
        # in the system (2)
        for container in (False, True):
            m, sf, d0, d1, c = make_test_pair_score(
                with_container=container, with_charge0=True,
                with_charge1=False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              sf.evaluate, False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              c.check_indexes, m, [d0, d1])

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_charge_invalid(self):
        """Test handling of Model with invalid charge attribute"""
        # We only add charge to the second particle, so the charge for
        # the first particle will be invalid
        for container in (False, True):
            m, sf, d0, d1, c = make_test_pair_score(
                with_container=container, with_charge0=False,
                with_charge1=True)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              sf.evaluate, False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              c.check_indexes, m, [d0, d1])


if __name__ == '__main__':
    IMP.test.main()
