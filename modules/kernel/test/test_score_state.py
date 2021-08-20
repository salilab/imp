from __future__ import print_function
import sys
import IMP
import IMP.test


class LoggingScoreState(IMP.ScoreState):

    """ScoreState that logs all calls"""

    def __init__(self, m, log, inputs=[]):
        IMP.ScoreState.__init__(self, m, "Logging%1%")
        self.log = log
        self.inputs = inputs

    def do_before_evaluate(self):
        self.log.append('update')

    def do_show(self, out):
        print("Hi")

    def get_type_name(self):
        return "LoggingScoreState"

    def do_after_evaluate(self, accum):
        self.log.append('after_evaluate')

    def do_show(self, fh=sys.stdout):
        fh.write("LoggingScoreState")

    def get_version_info(self):
        return IMP.get_module_version_info()

    def do_get_inputs(self):
        return self.inputs

    def do_get_outputs(self):
        return []


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
        self.assertEqual(IMP.get_dependent_score_states(m, p1), [ss1])
        self.assertEqual(IMP.get_dependent_score_states(m, p2), [])


if __name__ == '__main__':
    IMP.test.main()
