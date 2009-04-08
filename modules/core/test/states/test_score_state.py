import unittest
import IMP
import IMP.test

class LoggingRestraint(IMP.Restraint):
    """Do-nothing restraint that logs all calls"""

    def __init__(self, log):
        IMP.Restraint.__init__(self)
        self.log = log
    def show(self, junk=1):
        print "LoggingRestraint"
    def get_version_info(self):
        return IMP.VersionInfo("Ben Webb", "0.1")

    def evaluate(self, accum):
        if accum:
            self.log.append('restraint-deriv')
        else:
            self.log.append('restraint-score')
        return 0.0

class LoggingScoreState(IMP.ScoreState):
    """ScoreState that logs all calls"""

    def __init__(self, log):
        IMP.ScoreState.__init__(self)
        self.log = log

    def do_before_evaluate(self):
        self.log.append('update')

    def do_after_evaluate(self, accum):
        self.log.append('after_evaluate')
    def show(self, junk=1):
        print "LoggingRestraint"
    def get_version_info(self):
        return IMP.VersionInfo("Ben Webb", "0.1")


class TestScoreState(IMP.test.TestCase):
    """Tests for ScoreState"""

    def test_score_evaluate(self):
        """Make sure that ScoreState methods are called at score evaluation"""
        log = []
        model = IMP.Model()
        r = LoggingRestraint(log)
        model.add_restraint(r)
        ss = LoggingScoreState(log)
        model.add_score_state(ss)
        model.evaluate(False)
        self.assertEqual(log, ['update', 'restraint-score', 'after_evaluate'])
        log[:] = []
        model.evaluate(True)
        self.assertEqual(log, ['update', 'restraint-deriv', 'after_evaluate'])

    def test_score_state_show(self):
        """Test ScoreState::show()"""
        log = []
        ss = LoggingScoreState(log)
        ss.show()

if __name__ == '__main__':
    unittest.main()
