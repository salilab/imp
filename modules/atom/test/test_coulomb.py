import unittest
import random
import IMP
import IMP.test
import IMP.atom
import IMP.core

def make_test_pair_score():
    m = IMP.Model()
    p0 = IMP.Particle(m)
    d0 = IMP.atom.Charged.setup_particle(p0, IMP.algebra.Vector3D(0,0,0), 0.0)
    p1 = IMP.Particle(m)
    d1 = IMP.atom.Charged.setup_particle(p1, IMP.algebra.Vector3D(0,0,0), 0.0)
    c = IMP.atom.CoulombPairScore()
    r = IMP.core.PairRestraint(c, IMP.ParticlePair(p0, p1))
    m.add_restraint(r)
    return m, d0, d1, c


class CoulombTests(IMP.test.TestCase):
    """Test the CoulombPairScore"""

    def test_get_set(self):
        """Check CoulombPairScore get/set methods"""
        c = IMP.atom.CoulombPairScore()
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
                        v = IMP.algebra.random_vector_in_box(-box, box)
                        delta = IMP.algebra.random_vector_on_unit_sphere() \
                                * dist
                        d0.set_coordinates(v)
                        d1.set_coordinates(v + delta)
                        score = m.evaluate(False)
                        # Conversion factor to get score in kcal/mol
                        expected = 331.8469014486 * q0 * q1 / dist / eps
                        self.assertInTolerance(score, expected, 1e-2)

    def test_derivatives(self):
        """Check derivatives of CoulombPairScore"""
        m, d0, d1, c = make_test_pair_score()
        d0.set_charge(1.0)
        d1.set_charge(1.0)
        # Place one particle at the origin and the other at a random position
        # between 1 and 6 angstroms away (not too close since the derivatives
        # are too large there)
        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.random_vector_on_unit_sphere() \
                           * (random.random() * 5.0 + 1.0))
        self.assertXYZDerivativesInTolerance(m, d0, 1.0)
        self.assertXYZDerivativesInTolerance(m, d1, 1.0)

if __name__ == '__main__':
    unittest.main()
