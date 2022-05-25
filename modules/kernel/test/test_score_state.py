from __future__ import print_function
import sys
import IMP
import IMP.test


class LoggingScoreState(IMP.ScoreState):

    """ScoreState that logs all calls"""

    def __init__(self, m, log, can_skip=False, inputs=[], outputs=[],
                 suffix=''):
        IMP.ScoreState.__init__(self, m, "Logging%1%")
        self.set_can_skip(can_skip)
        self.log = log
        self.inputs = inputs
        self.outputs = outputs
        self.suffix = suffix

    def do_before_evaluate(self):
        self.log.append('update' + self.suffix)

    def do_show(self, out):
        print("Hi")

    def get_type_name(self):
        return "LoggingScoreState"

    def do_after_evaluate(self, accum):
        self.log.append('after_evaluate' + self.suffix)

    def do_show(self, fh=sys.stdout):
        fh.write("LoggingScoreState")

    def get_version_info(self):
        return IMP.get_module_version_info()

    def do_get_inputs(self):
        return self.inputs

    def do_get_outputs(self):
        return self.outputs


class Tests(IMP.test.TestCase):

    """Tests for ScoreState"""

    def test_score_state_update(self):
        """Test that score states are updated"""
        log = []
        m = IMP.Model()
        ss = LoggingScoreState(m, log)
        m.update()
        self.assertEqual(ss.log, ["update"])

    def test_get_dependent_score_states(self):
        """Test get_dependent_score_states() function"""
        log = []
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        ss1 = LoggingScoreState(m, log, inputs=[p1])
        self.assertEqual(m.get_dependent_score_states_uncached(p1), [ss1])
        self.assertEqual(m.get_dependent_score_states_uncached(p2), [])

    def test_evaluate_moved(self):
        """Test score state skipping in evaluate_moved()"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        p3 = IMP.Particle(m)
        p4 = IMP.Particle(m)
        p5 = IMP.Particle(m)
        p6 = IMP.Particle(m)
        log = []
        s12 = LoggingScoreState(m, log, inputs=[p1], outputs=[p2],
                                can_skip=True, suffix='s12')
        s2 = LoggingScoreState(m, log, inputs=[p2], outputs=[p6],
                               can_skip=True, suffix='s2')
        s3 = LoggingScoreState(m, log, inputs=[p3], outputs=[p6],
                               can_skip=True, suffix='s3')
        noskip = LoggingScoreState(m, log, inputs=[p4], outputs=[p6],
                                   can_skip=False, suffix='noskip')
        r = IMP._ConstRestraint(m, [p1, p2, p3, p4, p5, p6], 1)
        def assert_score_states(meth, exp_ss, *args):
            del log[:]
            meth(False, *args)
            exp_states = sorted(s.suffix for s in exp_ss)
            states = sorted(e[6:] for e in log if e.startswith('update'))
            self.assertEqual(states, exp_states)

        # Plain evaluate should call all ScoreStates
        assert_score_states(r.evaluate, [noskip, s12, s2, s3])

        # evaluate_moved with more than one particle should call all states
        assert_score_states(r.evaluate_moved, [noskip, s12, s2, s3], [p1, p2],
                            [])

        # p1 is direct input for s12 and indirect for s2
        assert_score_states(r.evaluate_moved, [noskip, s12, s2], [p1], [])

        # p2 is input for s2 and output for s21
        assert_score_states(r.evaluate_moved, [noskip, s12, s2], [p2], [])

        # p3 is input for s3
        assert_score_states(r.evaluate_moved, [noskip, s3], [p3], [])

        # p4 is input for noskip
        assert_score_states(r.evaluate_moved, [noskip], [p4], [])

        # p5 does not affect any can_skip state
        assert_score_states(r.evaluate_moved, [noskip], [p5], [])


if __name__ == '__main__':
    IMP.test.main()
