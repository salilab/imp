import unittest
import IMP
import IMP.test

class LoggingRestraint(IMP.Restraint):
    """Do-nothing restraint that logs all calls"""

    def __init__(self, log):
        IMP.Restraint.__init__(self)
        self.log = log
    def show(self, junk):
        print "LoggingRestraint"
    def get_version_info(self):
        return IMP.VersionInfo("Ben Webb", "0.1")

    def evaluate(self, accum):
        self.log.append('restraint')
        return 0.0

class LoggingScoreState(IMP.ScoreState):
    """ScoreState that logs all calls"""

    def __init__(self, log):
        IMP.ScoreState.__init__(self)
        self.log = log

    def update(self):
        self.log.append('update')

    def after_evaluate(self):
        self.log.append('after_evaluate')

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
        self.assertEqual(log, ['update', 'restraint', 'after_evaluate'])

    def test_score_state_show(self):
        """Test ScoreState::show()"""
        log = []
        ss = LoggingScoreState(log)
        ss.show()

if __name__ == '__main__':
    unittest.main()
