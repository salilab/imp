import unittest
import IMP
import IMP.test
import IMP.core

typekey = IMP.IntKey('mytype')

class TypedPairScoreTests(IMP.test.TestCase):
    """Class to test TypedPairScore"""

    def _make_particles(self, m, types):
        """Make particles with the given types"""
        ps = [IMP.Particle() for i in types]
        for p, typ in zip(ps, types):
            m.add_particle(p)
            p.add_attribute(typekey, typ)
        return ps

    def test_evaluate(self):
        """Check TypedPairScore::evaluate()"""
        ps = IMP.core.TypedPairScore(typekey)
        cps = IMP.test.ConstPairScore(5)
        ps.set_pair_score(cps, 0, 1)
        # Keep Python reference to the model so that the particles
        # aren't destroyed
        m = IMP.Model()
        pa, pb = self._make_particles(m, (0, 1))
        da = IMP.DerivativeAccumulator()
        # The ordering of the particles should not matter:
        self.assertEqual(ps.evaluate(pa, pb, da), 5.0)
        self.assertEqual(ps.evaluate(pb, pa, da), 5.0)

    def test_invalid_type(self):
        """Check TypedPairScore behavior with invalid particle types"""
        m = IMP.Model()
        pa, pb = self._make_particles(m, (0, 1))
        da = IMP.DerivativeAccumulator()
        ps1 = IMP.core.TypedPairScore(typekey, True)
        self.assertEqual(ps1.evaluate(pa, pb, da), 0.0)
        ps2 = IMP.core.TypedPairScore(typekey, False)
        self.assertRaises(ValueError, ps2.evaluate, pa, pb, da)

    def test_show(self):
        """Check TypedPairScore::show() method"""
        ps = IMP.core.TypedPairScore(typekey)
        ps.show()

if __name__ == '__main__':
    unittest.main()
