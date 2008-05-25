import unittest
import IMP
import IMP.test
import IMP.utils
import math

class One(IMP.UnaryFunction):
    def __init__(self):
        IMP.UnaryFunction.__init__(self)
    def evaluate(self, val):
        return 1.0
    def evaluate_deriv(self, val):
        return 1.0, 0.0
    def show(self, *args):
        print "identity"


class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def setUp(self):
        IMP.test.TestCase.setUp(self)

        # for some reason objects get destroyed even though
        # python doesn't own them
        self.stupid_hack=[]

    def create_particles(self, m, n):
        l0=IMP.Particles()
        for i in range(0,n):
            p= IMP.Particle()
            m.add_particle(p)
            d= IMP.XYZDecorator.create(p)
            l0.append(p)
        self.randomize_particles(l0, 20)
        return l0

    def create_angle_r(self, s, ps):
        print s.thisown
        for i in range(2, len(ps)):
            print str(i)
            l= One()
            self.stupid_hack.append(l)
            r= IMP.AngleRestraint(l, ps[i-2],ps[i-1], ps[i])
            s.add_restraint(r)
            #print r.evaluate(None)
            #print r.thisown
            #print l.thisown
            #print s.evaluate(None)
        print s.evaluate(None)

    def test_score(self):
        """Check score of chain triples restraints"""
        IMP.set_log_level(IMP.VERBOSE)
        m=IMP.Model()
        l0= self.create_particles(m, 3)
        l1= self.create_particles(m, 10)
        l2= self.create_particles(m, 2)
        l= One()
        t= IMP.AngleTripletScore(l)
        r= IMP.TripletChainRestraint(t)
        r.add_chain(l0)
        r.add_chain(l1)
        r.add_chain(l2)
        m.add_restraint(r)
        s= IMP.RestraintSet("angle restraints")
        m.add_restraint(s)
        print "creating angle restraints"
        self.create_angle_r(s, l0)
        self.create_angle_r(s, l1)
        self.create_angle_r(s, l2)
        ss= s.evaluate(None)
        rs= r.evaluate(None)
        diff = ss-rs
        print ss
        print rs
        if diff < 0:
            diff = -diff
        self.assert_(diff < .001, "The restraints are not equal")


if __name__ == '__main__':
    unittest.main()
