import unittest
import IMP.utils
import IMP.test, IMP

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def setUp(self):
        """Set up distance restraints"""
        IMP.test.TestCase.setUp(self)

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
        self.distrsr.append(IMP.DistanceRestraint(IMP.Harmonic(5.0, 0.1),
                                                  self.particles[0],
                                                  self.particles[1]))
        self.distrsr.append(IMP.DistanceRestraint(IMP.Harmonic(5.0, 0.1),
                                                  self.particles[1],
                                                  self.particles[2]))
        # add restraints
        self.rset = IMP.RestraintSet("distance_rsrs")
        self.model.add_restraint(self.rset)
        for d in self.distrsr:
            self.rset.add_restraint(d)

    def test_weights(self):
        """Test that sets can be weighted"""
        e1 = self.model.evaluate(True)
        d1 = self.particles[0].get_derivative(IMP.FloatKey("x"))
        self.rset.set_weight(0.5)
        e2 = self.model.evaluate(True)
        d2 = self.particles[0].get_derivative(IMP.FloatKey("x"))
        self.assertAlmostEqual(e1 * 0.5, e2, places=3)
        self.assertAlmostEqual(d1 * 0.5, d2, places=3)

    def test_interacting_particles(self):
        """Check RestraintSet::get_interacting_particles()"""
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

if __name__ == '__main__':
    unittest.main()
