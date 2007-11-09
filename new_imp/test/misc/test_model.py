import unittest
import IMP
import IMP.test

class DummyRestraint(IMP.Restraint):
    """Dummy do-nothing restraint"""
    def evaluate(self, accum):
        return 0.

class DummyState(IMP.State):
    """Dummy do-nothing state"""
    def update(self):
        pass

class ModelTests(IMP.test.IMPTestCase):
    def test_state(self):
        """Check state methods"""
        m = IMP.Model()
        self.assertRaises(IndexError, m.get_state, IMP.StateIndex(0));
        s = DummyState()
        m.add_state(s)
        news = m.get_state(IMP.StateIndex(0))
        self.assert_(isinstance(news, IMP.State))
        self.assertRaises(IndexError, m.get_state, IMP.StateIndex(1));

    def test_restraints(self):
        """Check restraint methods"""
        m = IMP.Model()
        self.assertRaises(IndexError, m.get_restraint, IMP.RestraintIndex(0));
        self.assertEqual(m.number_of_restraints(), 0)
        r = DummyRestraint()
        m.add_restraint(r)
        self.assertEqual(m.number_of_restraints(), 1)
        newr = m.get_restraint(IMP.RestraintIndex(0))
        self.assert_(isinstance(newr, IMP.Restraint))
        self.assertRaises(IndexError, m.get_restraint, IMP.RestraintIndex(1));

    def test_particles(self):
        """Check particle methods"""
        m = IMP.Model()
        p = IMP.Particle()
        self.assertEqual(m.number_of_particles(), 0)
        pi = m.add_particle(p)
        self.assertEqual(pi, IMP.ParticleIndex(0))
        self.assertEqual(m.number_of_particles(), 1)
        self.assertNotEqual(m.get_particle(IMP.ParticleIndex(0)), None)
        self.assertEqual(m.get_particle(IMP.ParticleIndex(1)), None)

if __name__ == '__main__':
    unittest.main()
