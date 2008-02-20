import unittest
import IMP, IMP.test

class IndexDiff(IMP.PairScore):
    def __init__(self):
        IMP.PairScore.__init__(self)
    def evaluate(self, pa, pb, da):
        d= (pa.get_index().get_index()- pb.get_index().get_index())
        print d
        return abs(d)
    def last_modified_by(self):
        return "Me"
    def version(self):
        return "0.5"
    def show(self, t):
        print "One Singleton"

class Linear(IMP.UnaryFunction):
    def __init__(self):
        IMP.UnaryFunction.__init__(self)
    def __call__(self, *args):
        return args[0]
    def show(self, *args):
        print "identity"

class TestPairList(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.VERBOSE)

    def test_it(self):
        """Test the pair list restraint"""
        m= IMP.Model()
        ps= IMP.ParticlePairs()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            if (i %2 == 1):
                ps.append(IMP.ParticlePair(p, last))
            last= p
        os= IndexDiff()
        s= IMP.PairListRestraint(ps, os)
        m.add_restraint(s)
        score= m.evaluate(False)
        print str(score)
        self.assertEqual(score, 5, "Wrong score")
        s.clear_particle_pairs()
        self.assertEqual(m.evaluate(False), 0, "Should be no terms")


if __name__ == '__main__':
    unittest.main()
