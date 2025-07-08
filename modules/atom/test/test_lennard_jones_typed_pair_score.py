import io
import random
import math
import IMP
import IMP.test
import IMP.atom
import IMP.core

from test_coulomb import place_xyzs


def make_test_pair_score(min_distance=9.0, max_distance=10.0,
                         with_container=False,
                         with_type0=True,
                         with_type1=True):
    m = IMP.Model()
    p0 = m.add_particle("p0")
    sph = IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), 4.0)
    IMP.core.XYZR.setup_particle(m, p0, sph)

    if with_type0:
        t0 = IMP.atom.LennardJonesType(1.0, 1.0, "type0")
        d0 = IMP.atom.LennardJonesTyped.setup_particle(m, p0, t0)
    else:
        t0 = None
        d0 = p0

    p1 = m.add_particle("p1")
    IMP.core.XYZR.setup_particle(m, p1, sph)

    if with_type1:
        t1 = IMP.atom.LennardJonesType(1.0, 1.0, "type1")
        d1 = IMP.atom.LennardJonesTyped.setup_particle(m, p1, t1)
    else:
        t1 = None
        d1 = p1

    sm = IMP.atom.ForceSwitch(min_distance, max_distance)
    c = IMP.atom.LennardJonesTypedPairScore(sm)
    if with_container:
        lpc = IMP.container.ListPairContainer(m, [(p0, p1)])
        r = IMP.container.PairsRestraint(c, lpc)
    else:
        r = IMP.core.PairRestraint(m, c, (p0, p1))
    sf = IMP.core.RestraintsScoringFunction([r])
    return m, sf, t0, t1, d0, d1, c


class Tests(IMP.test.TestCase):

    """Test the LennardJonesTypedPairScore"""

    def test_get_set(self):
        """Check LennardJonesTypedPairScore get/set methods"""
        sm = IMP.atom.ForceSwitch(9.0, 10.0)
        c = IMP.atom.LennardJonesTypedPairScore(sm)
        self.assertEqual(c.get_repulsive_weight(), 1.0)
        c.set_repulsive_weight(5.0)
        self.assertEqual(c.get_repulsive_weight(), 5.0)
        self.assertEqual(c.get_attractive_weight(), 1.0)
        c.set_attractive_weight(10.0)
        self.assertEqual(c.get_attractive_weight(), 10.0)

    def test_value(self):
        """Check score value of LennardJonesTypedPairScore"""
        m, sf, t0, t1, d0, d1, c = make_test_pair_score()

        box = IMP.algebra.Vector3D(10.0, 20.0, 30.0)
        for r0 in (2.0, 1.0):
            t0.set_radius(r0)
            for r1 in (2.0, 1.0):
                t1.set_radius(r1)
                rmin = r0 + r1
                for wd0 in (0.0, 1.0, 2.0):
                    t0.set_well_depth(wd0)
                    for wd1 in (0.0, 1.0, 2.0):
                        t1.set_well_depth(wd1)
                        wd = math.sqrt(wd0 * wd1)
                        for att in (0.0, 0.5, 1.0):
                            c.set_attractive_weight(att)
                            for rep in (0.0, 0.5, 1.0):
                                c.set_repulsive_weight(rep)
                                for r in (3.0, 4.0, 5.0):
                                    place_xyzs(d0, d1, box, r)
                                    score = sf.evaluate(False)
                                    expected = wd * (rep * (rmin / r) ** 12
                                                     - 2.0 * att * (rmin / r) ** 6)
                                    self.assertAlmostEqual(score, expected,
                                                           delta=1e-2)

    def test_derivatives(self):
        """Check derivatives of LennardJonesTypedPairScore"""
        m, sf, t0, t1, d0, d1, c = make_test_pair_score(4.0, 6.0)
        # Place one particle at the origin and the other at a random position
        # between 1 and 6 angstroms away (not too close since the derivatives
        # are too large there)
        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        d1.set_coordinates(IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
                           * (random.random() * 5.0 + 1.0))
        self.assertXYZDerivativesInTolerance(sf, d0, 2.0, 5.0)
        self.assertXYZDerivativesInTolerance(sf, d1, 2.0, 5.0)

    def test_smoothing(self):
        """Check smoothing of LennardJonesTypedPairScore"""
        m, sf, t0, t1, d0, d1, c = make_test_pair_score()
        smm, smsf, t0, t1, smd0, smd1, smc = make_test_pair_score(
            min_distance=4.0, max_distance=5.0)
        box = IMP.algebra.Vector3D(10.0, 20.0, 30.0)

        def place_all(dist):
            place_xyzs(d0, d1, box, dist)
            smd0.set_coordinates(d0.get_coordinates())
            smd1.set_coordinates(d1.get_coordinates())

        # For dist <= min_distance, scores should be identical
        for dist in (3.0, 3.5, 4.0):
            place_all(dist)
            self.assertAlmostEqual(sf.evaluate(False), smsf.evaluate(False),
                                   delta=1e-6)

        # For dist > max_distance, smoothed score should be zero
        place_all(5.5)
        self.assertEqual(smsf.evaluate(False), 0.0)
        self.assertNotEqual(sf.evaluate(False), 0.0)

    def test_cast(self):
        """Test LennardJonesTypedPairScore.get_from()"""
        sm = IMP.atom.ForceSwitch(7.0, 9.0)
        c = IMP.atom.LennardJonesTypedPairScore(sm)
        c.set_repulsive_weight(42.0)
        m = IMP.Model()
        # Get pairscore back as a generic Object
        ok = IMP.ModelKey("ljps")
        m.add_data(ok, c)
        r = m.get_data(ok)
        # Should be able to cast Object back to our LJ PairScore
        x = IMP.atom.LennardJonesTypedPairScore.get_from(r)
        self.assertIsInstance(x, IMP.atom.LennardJonesTypedPairScore)
        self.assertAlmostEqual(x.get_repulsive_weight(), 42.0, delta=1e-6)

        # Should not be able to cast some other object (Model)
        self.assertRaises(ValueError,
                          IMP.atom.LennardJonesTypedPairScore.get_from, m)

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_no_model_type_attribute(self):
        """Test handling of Model with no type attribute"""
        for container in (False, True):
            m, sf, t0, t1, d0, d1, c = make_test_pair_score(
                with_container=container, with_type0=False,
                with_type1=False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              sf.evaluate, False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              c.check_indexes, m, [d0, d1])

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_type_out_of_range(self):
        """Test handling of Model with type attribute out of range"""
        # We only add type to the first particle, so the dimension of
        # the type array will be 1, not the full number of particles
        # in the system (2)
        for container in (False, True):
            m, sf, t0, t1, d0, d1, c = make_test_pair_score(
                with_container=container, with_type0=True,
                with_type1=False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              sf.evaluate, False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              c.check_indexes, m, [d0, d1])

    @IMP.test.skipIf(IMP.get_check_level() < IMP.USAGE,
                     "No check in fast mode")
    def test_type_invalid(self):
        """Test handling of Model with invalid type attribute"""
        # We only add type to the second particle, so the type for
        # the first particle will be invalid
        for container in (False, True):
            m, sf, t0, t1, d0, d1, c = make_test_pair_score(
                with_container=container, with_type0=False,
                with_type1=True)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              sf.evaluate, False)
            self.assertRaises((IMP.InternalException, IMP.UsageException),
                              c.check_indexes, m, [d0, d1])


if __name__ == '__main__':
    IMP.test.main()
