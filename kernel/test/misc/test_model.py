import unittest
import IMP
import IMP.test
import StringIO

class DummyRestraint(IMP.Restraint):
    """Dummy do-nothing restraint"""
    def evaluate(self, accum):
        return 0.
    def show(self, something):
        print "I can't really show from python"
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")

class CustomError(Exception):
    pass

class FailingRestraint(IMP.Restraint):
    """Restraint that fails in evaluate"""
    def evaluate(self, accum):
        raise CustomError("Custom error message")
    def show(self, something):
        print "I can't really show from python"
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")

class DummyScoreState(IMP.ScoreState):
    """Dummy do-nothing score state"""
    def update(self):
        pass
    def show(self, something):
        print "I can't really show from python"
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")

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

    def test_show(self):
        """Check Model.show() method"""
        class BrokenFile(object):
            def write(self, str):
                raise NotImplementedError()
        m = IMP.Model()
        self.assertRaises(NotImplementedError, m.show, BrokenFile())
        self.assertRaises(AttributeError, m.show, None)
        s = StringIO.StringIO()
        m.show(s)
        self.assertEqual(s.getvalue()[:17], "\n\nModel:\nversion:")

    def test_refcount_director_score_state(self):
        """Refcounting should prevent director ScoreStates from being deleted"""
        dirchk = IMP.test.DirectorObjectChecker(self)
        m = IMP.Model()
        s = DummyScoreState()
        s.python_member = 'test string'
        m.add_score_state(s)
        # Since C++ now holds a reference to s, it should be safe to delete the
        # Python object (director objects should not be freed while C++ holds
        # a reference)
        del s
        news = m.get_score_state(0)
        self.assertEqual(news.python_member, 'test string')
        # Make sure we kept a reference somewhere to this director object
        dirchk.assert_number(1)
        # Cleanup should touch nothing as long as we have Python reference news
        dirchk.assert_number(1)
        del news
        # Should also touch nothing as long as we have C++ references (from m)
        dirchk.assert_number(1)
        del m
        # No refs remain, so make sure all director objects are cleaned up
        dirchk.assert_number(0)

    def test_director_python_exceptions(self):
        """Check that exceptions raised in directors are handled"""
        m = IMP.Model()
        r = FailingRestraint()
        m.add_restraint(r)
        self.assertRaises(CustomError, m.evaluate, False)

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

    def test_refcount_director_restraints(self):
        """Refcounting should prevent director Restraints from being deleted"""
        dirchk = IMP.test.DirectorObjectChecker(self)
        m = IMP.Model()
        r = DummyRestraint()
        r.python_member = 'test string'
        m.add_restraint(r)
        # Since C++ now holds a reference to r, it should be safe to delete the
        # Python object (director objects should not be freed while C++ holds
        # a reference)
        del r
        newr = m.get_restraint(0)
        self.assertEqual(newr.python_member, 'test string')
        # Make sure that all director objects are cleaned up
        dirchk.assert_number(1)
        del newr, m
        dirchk.assert_number(0)

    def test_particles(self):
        """Check particle methods"""
        m = IMP.Model()
        p = IMP.Particle(m)
        self.assertEqual(m.get_number_of_particles(), 1)
        for s in m.get_particles():
            s.show()

if __name__ == '__main__':
    unittest.main()
