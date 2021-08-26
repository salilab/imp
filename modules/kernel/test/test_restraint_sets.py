from __future__ import print_function
import IMP
import IMP.core
import IMP.test

class LinkScoreState(IMP.ScoreState):
    """ScoreState that links one particle to another"""

    def __init__(self, m, pin, pout):
        IMP.ScoreState.__init__(self, m, "LinkScoreState%1%")
        self.pin, self.pout = pin, pout

    def do_before_evaluate(self):
        pass

    def do_after_evaluate(self, accum):
        pass

    def do_get_inputs(self):
        return [self.pin]

    def do_get_outputs(self):
        return [self.pout]


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

    """Test RestraintSets"""

    def _make_stuff(self):
        m = IMP.Model("restraint sets")
        rs = IMP.RestraintSet(m, .5, "RS.5")
        r0 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r0)
        r1 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r1)
        r2 = IMP._ConstRestraint(m, [], 1)
        return (m, rs, r0, r1, r2)

    def test_printing(self):
        """Test that sets can be printed"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        sf = IMP.core.RestraintsScoringFunction([rs, r2])
        self.assertEqual(sf.evaluate(False), 2)
        print(rs)

    def test_restraints(self):
        """Check access to RestraintSet's restraints"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        rsrs = rs.get_restraints()
        self.assertEqual(rsrs.count(r0), 1)
        self.assertEqual(rsrs.count(r1), 1)
        self.assertEqual(rsrs.count(r2), 0)

    def test_restraints_weights(self):
        """Check weights on restraints"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        self.assertEqual(rs.evaluate(False), 1)
        self.assertEqual(r0.evaluate(False), 1)
        r0.set_weight(0)
        self.assertEqual(r0.evaluate(False), 0)
        self.assertEqual(rs.evaluate(False), .5)

    def test_evaluate(self):
        """Test evaluate() of RestraintSets and their children"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        self.assertEqual(rs.evaluate(False), 1)
        self.assertEqual(r0.evaluate(False), 1)

    def test_evaluate_2(self):
        """Test restraints added multiple times"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        sf = IMP.core.RestraintsScoringFunction([rs, r2, r0])
        self.assertEqual(sf.evaluate(False), 3)

    def test_removed(self):
        """Test that restraints are usable after set is destroyed"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        print("print removed", r1.evaluate(False))
        del rs
        print("destroyed", r1.evaluate(False))

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
        ss = LinkScoreState(m, p3, p4)
        m.add_score_state(ss)
        r1 = LogRestraint(m, [p1], 100.0)
        r4 = LogRestraint(m, [p4], 10.0)
        rs = IMP.RestraintSet(m, 1.0, 'rs')
        innerrs = IMP.RestraintSet(m, 1.0, 'rs')
        innerrs.add_restraints([r1, r4])
        rs.add_restraint(innerrs)
        def clear_restraints():
            r1.moved_evaluate = r4.moved_evaluate = None
        # Plain evaluate should *not* call the evaluate_moved method on r1,r4
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate(False), 110.0, delta=1e-6)
        assert_restraint_evaluate(r1)
        assert_restraint_evaluate(r4)

        # If we move p1, r1 should be evaluate_moved, r4 skipped
        # (and score should be unchanged, of course)
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p1], []),
                               110.0, delta=1e-6)
        assert_restraint_evaluate_moved(r1)
        assert_restraint_skipped(r4)

        # No restraints depend on p2
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p2], []),
                               110.0, delta=1e-6)
        assert_restraint_skipped(r1)
        assert_restraint_skipped(r4)

        # If we move p3, p4 is touched (by the LinkScoreState) so r4 should
        # be evaluate_moved
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p3], []),
                               110.0, delta=1e-6)
        assert_restraint_skipped(r1)
        assert_restraint_evaluate_moved(r4)

        # p4 causes r4 to be updated
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p4], []),
                               110.0, delta=1e-6)
        assert_restraint_skipped(r1)
        assert_restraint_evaluate_moved(r4)

        # Moves of multiple particles are not currently handled and will
        # fall back to plain evaluate
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p1, p2], []),
                               110.0, delta=1e-6)
        assert_restraint_evaluate(r1)
        assert_restraint_evaluate(r4)

        # Moves with derivatives are not currently handled and will
        # fall back to plain evaluate
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(True, [p1], []),
                               110.0, delta=1e-6)
        assert_restraint_evaluate(r1)
        assert_restraint_evaluate(r4)

        # Changes in weights should be handled
        innerrs.set_weight(2.)
        self.assertAlmostEqual(rs.evaluate_moved(False, [p1], []),
                               220.0, delta=1e-6)
        innerrs.set_weight(1.)

        # Changing weight of an evaluated restraint should be OK
        r1.set_weight(2.)
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p1], []),
                               210.0, delta=1e-6)
        assert_restraint_evaluate_moved(r1)
        assert_restraint_skipped(r4)

        # Changing weight of a skipped restraint should be OK
        clear_restraints()
        self.assertAlmostEqual(rs.evaluate_moved(False, [p4], []),
                               210.0, delta=1e-6)
        assert_restraint_skipped(r1)
        assert_restraint_evaluate_moved(r4)

        # Cache should be cleared since we changed the dependency graph here;
        # now moving p1 should force recalc of r1 *and* r4
        clear_restraints()
        ss2 = LinkScoreState(m, p1, p4)
        m.add_score_state(ss2)
        self.assertAlmostEqual(rs.evaluate_moved(False, [p1], []),
                               210.0, delta=1e-6)
        assert_restraint_evaluate_moved(r1)
        assert_restraint_evaluate_moved(r4)

        # Newly-added restraints should use regular evaluate to get first score
        clear_restraints()
        r3 = LogRestraint(m, [p3], 30.0)
        r3.moved_evaluate = None
        innerrs.add_restraint(r3)
        self.assertAlmostEqual(rs.evaluate_moved(False, [p4], []),
                               240.0, delta=1e-6)
        assert_restraint_skipped(r1)
        assert_restraint_evaluate(r3)
        assert_restraint_evaluate_moved(r4)


if __name__ == '__main__':
    IMP.test.main()
