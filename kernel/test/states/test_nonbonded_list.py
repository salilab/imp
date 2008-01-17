import unittest
import IMP, IMP.test

class OnePair(IMP.PairScore):
    def __init__(self):
        IMP.PairScore.__init__(self)
    def evaluate(self, pa, pb, da):
        return 1
    def last_modified_by(self):
        return "Me"
    def version(self):
        return "0.5"
    def show(self, t):
        print "One Pair"


class TestNBL(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.TERSE)

    def test_it(self):
        """Test the nonbonded list and restraint which uses it"""
        m= IMP.Model()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.set_x(0)
            d.set_y(1)
            d.set_z(1)
        s= IMP.NonbondedListScoreState(m.get_particles(), 100)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(s, o)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 45, "Wrong score")


if __name__ == '__main__':
    unittest.main()
