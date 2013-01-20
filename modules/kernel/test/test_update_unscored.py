import sys
import IMP
import IMP.test


class TouchSS(IMP.ScoreState):
    """ScoreState that logs all calls"""

    def __init__(self, m, pi, k):
        IMP.ScoreState.__init__(self, m)
        self.pi=pi
        self.k=k
    def do_before_evaluate(self):
        self.get_model().set_attribute(self.k, self.pi, 1)
    def get_type_name(self):
        return "TouchSS"
    def do_after_evaluate(self, accum):
        pass
    def get_version_info(self):
        return IMP.get_module_version_info()
    def do_get_inputs(self):
        return []
    def do_get_outputs(self):
        return [self.get_model().get_particle(self.pi)]

class Tests(IMP.test.TestCase):
    """Tests for ScoreState"""

    def test_score_state_update(self):
        """Test that unscored particles are updated on evaluate"""
        m=IMP.Model()
        pi= m.add_particle("my particle")
        k= IMP.FloatKey("hi")
        ss = TouchSS(m, pi, k)
        m.add_attribute(k, pi, 0)
        m.set_is_optimized(k, pi, True)
        m.evaluate(False)
        self.assertEqual(m.get_attribute(k, pi), 1)
if __name__ == '__main__':
    IMP.test.main()
