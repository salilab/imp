from __future__ import print_function
import IMP
import IMP.algebra
import IMP.core
import IMP.test


class Tests(IMP.test.TestCase):

    def test_changed(self):
        """Test DistanceToSingletonScore.get_any_particle_changed()"""
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(1,1,1))
        u = IMP.core.Harmonic(0, 1)
        s = IMP.core.DistanceToSingletonScore(u, IMP.algebra.Vector3D(0,0,0))
        r = IMP.core.SingletonRestraint(m, s, p)
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.5, delta=1e-6)
        self.assertFalse(s.get_any_particle_changed(m, p))
        # this should reuse the cached value
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.5, delta=1e-6)
        # derivatives aren't cached, so this won't use the cache
        score = r.evaluate(True)
        self.assertAlmostEqual(score, 1.5, delta=1e-6)
        d.set_coordinates(IMP.algebra.Vector3D(2,2,2))
        self.assertTrue(m.get_has_particle_changed(p))
        self.assertTrue(s.get_any_particle_changed(m, p))
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 6.0, delta=1e-6)
        self.assertFalse(s.get_any_particle_changed(m, p))

if __name__ == '__main__':
    IMP.test.main()
