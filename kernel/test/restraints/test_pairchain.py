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

    def test_it(self):
        """Test the pair list restraint"""
        m= IMP.Model()
        ps0= IMP.Particles()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            ps0.append(p)
        ps1= IMP.Particles()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            ps1.append(p)
        os= IndexDiff()
        s= IMP.PairChainRestraint(os)
        s.add_chain(ps0)
        s.add_chain(ps1)
        m.add_restraint(s)
        score= m.evaluate(False)
        print str(score)
        self.assertEqual(score, 18, "Wrong score")
        s.clear_chains()
        self.assertEqual(m.evaluate(False), 0, "Should be no terms")


if __name__ == '__main__':
    unittest.main()
