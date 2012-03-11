import sys
import IMP
import IMP.test

class TestScoreState(IMP.test.TestCase):
    """Tests for ScoreState"""

    def test_score_state_show(self):
        """Test scoring function linkage"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        r= IMP._ConstRestraint(1, ps)
        r.set_model(m)
        r.create_scoring_function()
        self.assertEqual(r.evaluate(False), 1)

if __name__ == '__main__':
    IMP.test.main()
