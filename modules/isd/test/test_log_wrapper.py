import IMP
import IMP.isd
import IMP.test
import pickle


class LogRestraint(IMP.Restraint):
    """Restraint that logs how it was called (or skipped)"""

    def __init__(self, m, ps, value):
        IMP.Restraint.__init__(self, m, 'LogRestraint%1%')
        self.ps = ps
        self.value = value
        self.moved_evaluate = None

    def unprotected_evaluate(self, accum):
        self.moved_evaluate = False
        return self.value

    def unprotected_evaluate_moved(self, accum, moved_pis, reset_pis):
        self.moved_evaluate = True
        return self.value

    def do_get_inputs(self):
        return self.ps


class Tests(IMP.test.TestCase):

    def test_evaluate_moved(self):
        """Test scoring with moved particles"""
        def assert_restraint_skipped(r):
            # In debug mode restraints aren't actually skipped; they
            # evaluated and the score is checked against the 'skipped' score
            if IMP.get_check_level() >= IMP.USAGE_AND_INTERNAL:
                assert_restraint_evaluate_moved(r)
            else:
                self.assertIsNone(r.moved_evaluate)
        def assert_restraint_evaluate(r):
            self.assertFalse(r.moved_evaluate)
        def assert_restraint_evaluate_moved(r):
            self.assertTrue(r.moved_evaluate)
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        p3 = IMP.Particle(m)
        p4 = IMP.Particle(m)
        r1 = LogRestraint(m, [p1], 100.0)
        r4 = LogRestraint(m, [p4], 10.0)
        lw = IMP.isd.LogWrapper([r1, r4], 1.0)
        def clear_restraints():
            r1.moved_evaluate = r4.moved_evaluate = None
        # Plain evaluate should *not* call the evaluate_moved method on r1,r4
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate(False), -6.908, delta=1e-3)
        assert_restraint_evaluate(r1)
        assert_restraint_evaluate(r4)

        # If we move p1, r1 should be evaluate_moved, r4 skipped
        # (and score should be unchanged, of course)
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p1], []),
                               -6.908, delta=1e-3)
        assert_restraint_evaluate_moved(r1)
        assert_restraint_skipped(r4)

        # No restraints depend on p2
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p2], []),
                               -6.908, delta=1e-3)
        assert_restraint_skipped(r1)
        assert_restraint_skipped(r4)

        # p4 causes r4 to be updated
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p4], []),
                               -6.908, delta=1e-3)
        assert_restraint_skipped(r1)
        assert_restraint_evaluate_moved(r4)

        # Moves of multiple particles are not specially handled but will
        # pass through to evaluate_moved
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p1, p2], []),
                               -6.908, delta=1e-3)
        assert_restraint_evaluate_moved(r1)
        assert_restraint_evaluate_moved(r4)

        # Moves with derivatives are not specially handled but will
        # pass through to evaluate_moved
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(True, [p1], []),
                               -6.908, delta=1e-3)
        assert_restraint_evaluate_moved(r1)
        assert_restraint_evaluate_moved(r4)

        # Newly-added restraints should use regular evaluate to get first score
        clear_restraints()
        r3 = LogRestraint(m, [p3], 30.0)
        r3.moved_evaluate = None
        lw.add_restraint(r3)
        self.assertAlmostEqual(lw.evaluate_moved(False, [p4], []),
                               -10.308, delta=1e-3)
        assert_restraint_skipped(r1)
        assert_restraint_evaluate(r3)
        assert_restraint_evaluate_moved(r4)

    def test_aggregate(self):
        """Test get_is_aggregate()"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        r1 = LogRestraint(m, [p1], 100.0)
        r2 = LogRestraint(m, [p2], 10.0)
        lw = IMP.isd.LogWrapper([r1, r2], 1.0)
        self.assertFalse(r1.get_is_aggregate())
        self.assertTrue(lw.get_is_aggregate())

    def test_pickle(self):
        """Test that LogWrapper can be (un-)pickled"""
        m = IMP.Model()
        r0 = IMP._ConstRestraint(m, [], 100)
        lw = IMP.isd.LogWrapper([r0], 1.0)
        lw.set_name("foo")
        self.assertAlmostEqual(lw.evaluate(False), -4.605, delta=1e-3)
        dump = pickle.dumps(lw)
        newlw = pickle.loads(dump)
        self.assertEqual(newlw.get_name(), "foo")
        self.assertEqual([lw.get_name() for r in newlw.restraints], ["foo"])
        self.assertAlmostEqual(newlw.evaluate(False), -4.605, delta=1e-3)

    def test_pickle_polymorphic(self):
        """Test that LogWrapper can be (un-)pickled via polymorphic pointer"""
        m = IMP.Model()
        r0 = IMP._ConstRestraint(m, [], 100)
        lw = IMP.isd.LogWrapper([r0], 1.0)
        rs = IMP.RestraintSet(m, 12.0)
        rs.restraints.append(lw)
        self.assertAlmostEqual(rs.evaluate(False), -55.262, delta=1e-3)
        dump = pickle.dumps(rs)
        newrs = pickle.loads(dump)
        self.assertAlmostEqual(newrs.evaluate(False), -55.262, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
