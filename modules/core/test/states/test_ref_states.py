import unittest
import IMP
import IMP.test
import IMP.core

class RefCountTests(IMP.test.TestCase):
    """Test refcounting of ScoreStates"""

    def _make_objects(self):
        IMP.set_log_level(IMP.MEMORY)
        refcnt = IMP.test.RefCountChecker(self)
        print "m"
        m = IMP.Model()
        print "c"
        c = IMP.core.SingletonContainerSet()
        print "s"
        s = IMP.core.ClosePairsScoreState(c)
        m.add_score_state(s)
        # Reference to c is not kept
        return refcnt, m, s

    def test_simple(self):
        """Check reference counting of ScoreStates"""
        refcnt, m, s = self._make_objects()
        # ClosePairsScoreState also creates a PairContainer and
        # a ClosePairsFinder (and holds a reference to the SingletonContainer)
        # so 5 objects in total
        refcnt.assert_number(5)
        # Model should hold a ref to state, so nothing should be freed
        # until it is
        print "del s"
        del s
        refcnt.assert_number(5)
        print "del m"
        del m
        refcnt.assert_number(0)

    def test_delete_model_constructor(self):
        """Constructed Python states should survive model deletion"""
        refcnt, m, s = self._make_objects()
        self.assertEqual(s.get_ref_count(), 2)
        refcnt.assert_number(5)
        # New state s should not go away until we free the Python reference
        del m
        refcnt.assert_number(4)
        self.assertEqual(s.get_ref_count(), 1)
        del s
        refcnt.assert_number(0)

    def test_delete_optimizer_accessor(self):
        "ScoreStates from vector accessors should survive Model deletion"
        refcnt, m, s = self._make_objects()
        del s
        # Now create new Python ScoreState s from a C++ vector accessor
        # These accessors call specific methods in the SWIG wrapper which
        # are modified by typemaps in our interface.
        s = m.get_score_states()[0]
        # Python reference s plus C++ reference from m
        self.assertEqual(s.get_ref_count(), 2)
        del m
        # Now only the Python reference s should survive
        self.assertEqual(s.get_ref_count(), 1)
        refcnt.assert_number(4)
        del s
        refcnt.assert_number(0)


if __name__ == '__main__':
    unittest.main()
