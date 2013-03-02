import IMP
import IMP.test
import IMP.core
import StringIO

typekey = IMP.IntKey('mytype')

class Tests(IMP.test.TestCase):
    """Class to test TypedPairScore"""

    def _make_particles(self, m, types):
        """Make particles with the given types"""
        ps = [IMP.Particle(m) for i in types]
        for p, typ in zip(ps, types):
            p.add_attribute(typekey, typ)
        return ps

    def test_evaluate(self):
        """Check TypedPairScore::evaluate()"""
        ps = IMP.core.TypedPairScore(typekey)
        cps = IMP.kernel._ConstPairScore(5)
        ps.set_pair_score(cps, 0, 1)
        # Keep Python reference to the model so that the particles
        # aren't destroyed
        m = IMP.Model()
        pa, pb = self._make_particles(m, (0, 1))
        da = IMP.DerivativeAccumulator()
        # The ordering of the particles should not matter:
        pab=(pa, pb)
        pba=(pb, pa)
        self.assertEqual(ps.evaluate(pab, da), 5.0)
        self.assertEqual(ps.evaluate(pba, da), 5.0)

    def test_invalid_type(self):
        """Check TypedPairScore behavior with invalid particle types"""
        m = IMP.Model()
        pa, pb = self._make_particles(m, (0, 1))
        da = IMP.DerivativeAccumulator()
        ps1 = IMP.core.TypedPairScore(typekey, True)
        self.assertEqual(ps1.evaluate((pa, pb), da), 0.0)
        ps2 = IMP.core.TypedPairScore(typekey, False)
        self.assertRaises(ValueError, ps2.evaluate, (pa, pb), da)
if __name__ == '__main__':
    IMP.test.main()
