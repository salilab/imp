import unittest
import IMP
import IMP.test

class DummyRestraint(IMP.Restraint):
    """Dummy do-nothing restraint"""
    def evaluate(self, accum):
        return 0.
    def show(self, something):
        print "I can't really show from python"


class DummyScoreState(IMP.ScoreState):
    """Dummy do-nothing score state"""
    def update(self):
        pass
    def show(self, something):
        print "I can't really show from python"

class ModelTests(IMP.test.TestCase):
    def test_score_state(self):
        """Check score state methods"""
        m = IMP.Model()
        self.assertRaises(IndexError, m.get_score_state,
                          0);
        s = DummyScoreState()
        m.add_score_state(s)
        news = m.get_score_state(0)
        self.assert_(isinstance(news, IMP.ScoreState))
        self.assertRaises(IndexError, m.get_score_state,
                          1);
        for s in m.get_score_states():
            s.show()

    def test_restraints(self):
        """Check restraint methods"""
        m = IMP.Model()
        self.assertRaises(IndexError, m.get_restraint, 0);
        self.assertEqual(m.get_number_of_restraints(), 0)
        r = DummyRestraint()
        m.add_restraint(r)
        self.assertEqual(m.get_number_of_restraints(), 1)
        newr = m.get_restraint(0)
        self.assert_(isinstance(newr, IMP.Restraint))
        self.assertRaises(IndexError, m.get_restraint,1);
        for s in m.get_restraints():
            s.show()


    def test_particles(self):
        """Check particle methods"""
        m = IMP.Model()
        p = IMP.Particle(m)
        self.assertEqual(m.get_number_of_particles(), 1)
        for s in m.get_particles():
            s.show()

if __name__ == '__main__':
    unittest.main()
