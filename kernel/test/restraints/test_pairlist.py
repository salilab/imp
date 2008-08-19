import unittest
import IMP, IMP.test

class IndexDiff(IMP.PairScore):
    def __init__(self):
        IMP.PairScore.__init__(self)
    def evaluate(self, pa, pb, da):
        d= (pa.get_index().get_index()- pb.get_index().get_index())
        print d
        return abs(d)
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def show(self, t):
        print "One Singleton"

class TestPairList(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)

    def _make_pairlist(self):
        m= IMP.Model()
        ps= IMP.ParticlePairs()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            if i % 2 == 1:
                ps.append(IMP.ParticlePair(p, last))
            last= p
        os= IndexDiff()
        s= IMP.PairListRestraint(os, ps)
        m.add_restraint(s)
        return m, ps, os, s

    def test_it(self):
        """Test the pair list restraint"""
        m, ps, os, s = self._make_pairlist()
        score= m.evaluate(False)
        print str(score)
        self.assertEqual(score, 5, "Wrong score")
        s.clear_particle_pairs()
        self.assertEqual(m.evaluate(False), 0, "Should be no terms")

    def test_interacting_particles(self):
        """Check PairListRestraint::get_interacting_particles()"""
        m, ps, os, s = self._make_pairlist()
        ipar = s.get_interacting_particles()
        # Should return a set of pairs from ps:
        self.assertEqual(len(ipar), 5)
        self.assertEqual(len(ipar), len(ps))
        for pipar, pps in zip(ipar, ps):
            self.assertEqual(len(pipar), 2)
            for a, b in zip(pipar, pps):
                self.assertEqual(a, b)

if __name__ == '__main__':
    unittest.main()
