from __future__ import print_function
import IMP
import IMP.isd
import IMP.test


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

    def unprotected_evaluate_moved(self, accum, moved_pis):
        self.moved_evaluate = True
        return self.value

    def do_get_inputs(self):
        return self.ps


class Tests(IMP.test.TestCase):

    def test_evaluate_moved(self):
        """Test scoring with moved particles"""
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
        self.assertFalse(r1.moved_evaluate)
        self.assertFalse(r4.moved_evaluate)

        # If we move p1, r1 should be evaluate_moved, r4 skipped
        # (and score should be unchanged, of course)
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p1]),
                               -6.908, delta=1e-3)
        self.assertTrue(r1.moved_evaluate)
        self.assertIsNone(r4.moved_evaluate)

        # No restraints depend on p2
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p2]),
                               -6.908, delta=1e-3)
        self.assertIsNone(r1.moved_evaluate)
        self.assertIsNone(r4.moved_evaluate)

        # p4 causes r4 to be updated
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p4]),
                               -6.908, delta=1e-3)
        self.assertIsNone(r1.moved_evaluate)
        self.assertTrue(r4.moved_evaluate)

        # Moves of multiple particles are not currently handled and will
        # fall back to plain evaluate
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(False, [p1, p2]),
                               -6.908, delta=1e-3)
        self.assertFalse(r1.moved_evaluate)
        self.assertFalse(r4.moved_evaluate)

        # Moves with derivatives are not currently handled and will
        # fall back to plain evaluate
        clear_restraints()
        self.assertAlmostEqual(lw.evaluate_moved(True, [p1]),
                               -6.908, delta=1e-3)
        self.assertFalse(r1.moved_evaluate)
        self.assertFalse(r4.moved_evaluate)


if __name__ == '__main__':
    IMP.test.main()
