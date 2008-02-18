import unittest
import IMP, IMP.test
import random

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

class OneScore(IMP.UnaryFunction):
    def __init__(self):
        IMP.UnaryFunction.__init__(self)
    def __call__(self, *args):
        return 100
    def show(self, *args):
        print "One score"

class TestNBL(IMP.test.TestCase):
    def setUp(self):
        IMP.set_log_level(IMP.VERBOSE)

    def test_it(self):
        """Test the nonbonded list and restraint which uses it"""
        m= IMP.Model()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.set_x(random.uniform(0,10))
            d.set_y(random.uniform(0,10))
            d.set_z(random.uniform(0,10))
        s= IMP.AllNonbondedListScoreState(m.get_particles())
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(s, o, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 45, "Wrong score")

    def test_bl(self):
        """Test the bonded list"""
        return
        m= IMP.Model()
        bds=[]
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.set_x(0)
            d.set_y(10)
            d.set_z(10)
            bds.append(IMP.BondedDecorator.create(p))
        pts=IMP.Particles()
        for p in m.get_particles():
            pts.append(p)
        for i in range(1,10):
            IMP.custom_bond(bds[i-1], bds[i], 1, .1)
        s= IMP.AllNonbondedListScoreState(pts, 1)
        b= IMP.BondDecoratorListScoreState(pts)
        s.add_bonded_list(b)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(s, o, 10)
        os=OneScore()
        print os(6)
        br= IMP.BondDecoratorRestraint(b, os)
        m.add_restraint(r)
        m.add_restraint(br)
        score= m.evaluate( False )
        self.assertEqual(score, 900+45-9, "Wrong score")

    def test_distfilt(self):
        """Test filtering based on distance in nonbonded list"""
        m= IMP.Model()
        ps=IMP.Particles()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            ps.append(p)
            if (i < 5):
                d.set_x(random.uniform(0,10))
                d.set_y(random.uniform(0,10))
                d.set_z(random.uniform(0,10))
            else:
                d.set_x(random.uniform(50,60))
                d.set_y(random.uniform(50,60))
                d.set_z(random.uniform(50,60))
        s= IMP.AllNonbondedListScoreState(ps)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(s, o, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 20, "Wrong score")
    def test_bi(self):
        """Test the bipartite nonbonded list and restraint which uses it"""
        m= IMP.Model()
        ps0=IMP.Particles()
        ps1=IMP.Particles()
        for i in range(0,10):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.XYZDecorator.create(p)
            d.set_x(random.uniform(0,10))
            d.set_y(random.uniform(0,10))
            d.set_z(random.uniform(0,10))
            if (i < 5):
                ps0.append(p)
            else:
                ps1.append(p)
        s= IMP.BipartiteNonbondedListScoreState(ps0, ps1, 1000)
        m.add_score_state(s)
        o= OnePair()
        r= IMP.NonbondedRestraint(s, o, 15)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 25, "Wrong score")

if __name__ == '__main__':
    unittest.main()
