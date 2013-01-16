import sys
import IMP
import IMP.test


class LoggingScoreState(IMP.ScoreState):
    """ScoreState that logs all calls"""

    def __init__(self, m, log):
        IMP.ScoreState.__init__(self, m)
        self.log = log

    def do_before_evaluate(self):
        self.log.append('update')
    def do_show(self, out):
        print "Hi"
    def get_type_name(self):
        return "LoggingScoreState"
    def do_after_evaluate(self, accum):
        self.log.append('after_evaluate')
    def do_show(self, fh=sys.stdout):
        fh.write("LoggingScoreState")
    def get_version_info(self):
        return IMP.get_module_version_info()
    def do_get_inputs(self):
        return []
    def do_get_outputs(self):
        return []

class Tests(IMP.test.TestCase):
    """Tests for ScoreState"""

    def test_score_state_update(self):
        """Test that score states are updated"""
        log = []
        m=IMP.Model()
        ss = LoggingScoreState(m, log)
        m.update()
        self.assertEqual(ss.log, ["update"])
if __name__ == '__main__':
    IMP.test.main()
