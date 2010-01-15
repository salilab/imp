import unittest
import IMP
import IMP.test
import IMP.core

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def _make_distance_restraints(self):
        self.model = IMP.Model()
        self.particles = []

        # create particles
        self.particles.append(self.create_point_particle(self.model,
                                                         -43.0, 65.0, 93.0))
        self.particles.append(self.create_point_particle(self.model,
                                                         20.0, 74.0, -80.0))
        self.particles.append(self.create_point_particle(self.model,
                                                         10.0, 38.0, 20.0))

        # separate particles by 5.0:
        self.distrsr = []
        d = IMP.core.DistanceRestraint(IMP.core.Harmonic(5.0, 0.1),
                                       self.particles[0], self.particles[1])
        self.distrsr.append(d)
        d = IMP.core.DistanceRestraint(IMP.core.Harmonic(5.0, 0.1),
                                       self.particles[1], self.particles[2])
        self.distrsr.append(d)
        # add restraints
        self.rset = IMP.core.RestraintSet("distance_rsrs")
        self.model.add_restraint(self.rset)
        for d in self.distrsr:
            self.rset.add_restraint(d)

    def test_weights(self):
        """Test that sets can be weighted"""
        self._make_distance_restraints()
        e1 = self.model.evaluate(True)
        d1 = self.particles[0].get_derivative(IMP.FloatKey("x"))
        self.rset.set_weight(0.5)
        e2 = self.model.evaluate(True)
        d2 = self.particles[0].get_derivative(IMP.FloatKey("x"))
        self.assertAlmostEqual(e1 * 0.5, e2, places=3)
        self.assertAlmostEqual(d1 * 0.5, d2, places=3)

    def test_interacting_particles(self):
        """Check RestraintSet::get_interacting_particles()"""
        self._make_distance_restraints()
        ipar = self.rset.get_interacting_particles()
        # Should return the union of the restraint particle sets
        # (i.e. two sets, each containing the particles from one restraint)
        self.assertEqual(len(ipar), 2)
        self.assertEqual(len(ipar), len(self.distrsr))
        for pipar, d in zip(ipar, self.distrsr):
            pdistrsr = d.get_interacting_particles()
            self.assertEqual(len(pdistrsr), 1)
            self.assertEqual(len(pipar), len(pdistrsr[0]))
            for a, b in zip(pipar, pdistrsr[0]):
                self.assertEqual(a, b)

    def test_restraints(self):
        """Check access to RestraintSet's restraints"""
        self._make_distance_restraints()
        r = self.rset
        rsrs = r.get_restraints()
        self.assertEqual(len(rsrs), len(self.distrsr))
        for n, val in enumerate(self.distrsr):
            self.assertEqual(r.get_restraint(n), val)
            self.assertEqual(rsrs[n], val)
        #self.assertRaises(IndexError, r.get_restraint, 2)

    def test_evaluate(self):
        """Test evaluate() of RestraintSets and their children"""
        m = IMP.Model()

        s = IMP.core.RestraintSet()
        s.set_weight(0.1)
        c1 = IMP.core.ConstantRestraint(10.0)
        c2 = IMP.core.ConstantRestraint(20.0)

        s.add_restraint(c1)
        s.add_restraint(c2)
        m.add_restraint(s)

        # Cannot currently call evaluate() on a Restraint inside a RestraintSet
        # (except in fast mode, where usage checks are disabled).
        if IMP.has_tests:
            self.assertRaises(IMP.UsageException, c1.evaluate, False)
            self.assertRaises(IMP.UsageException, c2.evaluate, False)
        self.assertEqual(s.evaluate(False), 3.0)

if __name__ == '__main__':
    unittest.main()
