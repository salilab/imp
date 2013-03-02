import StringIO
import random
import IMP
import IMP.test
import IMP.atom
import IMP.core

def make_test_pair_score(min_distance=9.0, max_distance=10.0):
    m = IMP.Model()
    p0 = IMP.Particle(m)
    d0 = IMP.atom.Charged.setup_particle(p0, IMP.algebra.Vector3D(0,0,0), 0.0)
    p1 = IMP.Particle(m)
    d1 = IMP.atom.Charged.setup_particle(p1, IMP.algebra.Vector3D(0,0,0), 0.0)
    sm = IMP.atom.ForceSwitch(min_distance, max_distance)
    c = IMP.atom.CoulombPairScore(sm)
    r = IMP.core.PairRestraint(c, (p0, p1))
    m.add_restraint(r)
    return m, d0, d1, c

def place_xyzs(xyz0, xyz1, box, dist):
    """Place two XYZ particles randomly a given distance apart"""
    v = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(-box, box))
    delta = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d()) * dist
    xyz0.set_coordinates(v)
    xyz1.set_coordinates(v + delta)


class Tests(IMP.test.TestCase):
    """Test the CoulombPairScore"""

    def test_get_set(self):
        """Check CoulombPairScore get/set methods"""
        sm = IMP.atom.ForceSwitch(9.0, 10.0)
        c = IMP.atom.CoulombPairScore(sm)
        self.assertEqual(c.get_relative_dielectric(), 1.0)
        c.set_relative_dielectric(5.0)
        self.assertEqual(c.get_relative_dielectric(), 5.0)

    def test_value(self):
        """Check score value of CoulombPairScore"""
        m, d0, d1, c = make_test_pair_score()

        box = IMP.algebra.Vector3D(10.0, 20.0, 30.0)
        for q0 in (2.0, -1.0, 0.0):
            d0.set_charge(q0)
            for q1 in (2.0, -1.0, 0.0):
                d1.set_charge(q1)
                for eps in (1.0, 5.0):
                    c.set_relative_dielectric(eps)
                    for dist in (0.5, 1.0, 2.0, 3.0):
                        place_xyzs(d0, d1, box, dist)
                        score = m.evaluate(False)
                        # Conversion factor to get score in kcal/mol
                        expected = 331.8469014486 * q0 * q1 / dist / eps
                        self.assertAlmostEqual(score, expected, delta=1e-2)

    def test_derivatives(self):
        """Check derivatives of CoulombPairScore"""
        m, d0, d1, c = make_test_pair_score(4.0, 6.0)
        d0.set_charge(1.0)
        d1.set_charge(1.0)
        # Place one particle at the origin and the other at a random position
        # between 1 and 6 angstroms away (not too close since the derivatives
        # are too large there)
        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d()) \
                           * (random.random() * 5.0 + 1.0))
        self.assertXYZDerivativesInTolerance(m, d0, 2.0, 3.0)
        self.assertXYZDerivativesInTolerance(m, d1, 2.0, 3.0)

    def test_smoothing(self):
        """Check smoothing of CoulombPairScore"""
        m, d0, d1, c = make_test_pair_score()
        smm, smd0, smd1, smc = make_test_pair_score(min_distance=4.0,
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
            self.assertAlmostEqual(m.evaluate(False), smm.evaluate(False),
                                   delta=1e-6)

        # For dist > max_distance, smoothed score should be zero
        place_all(5.5)
        self.assertEqual(smm.evaluate(False), 0.0)
        self.assertNotEqual(m.evaluate(False), 0.0)

if __name__ == '__main__':
    IMP.test.main()
