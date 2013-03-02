import sys
import IMP
import IMP.test

class Tests(IMP.test.TestCase):
    """Tests for ScoreState"""

    def test_score_state_show(self):
        """Test scoring function linkage"""
        m= IMP.Model("scoring function linkage")
        ps= [IMP.Particle(m) for i in range(0,10)]
        r= IMP.kernel._ConstRestraint(1, ps)
        r.set_model(m)
        r.create_scoring_function()
        self.assertEqual(r.evaluate(False), 1)
    def test_reweighting(self):
        """Test scoring function reweighting"""
        m= IMP.Model("scoring function linkage")
        ps= [IMP.Particle(m) for i in range(0,10)]
        r= IMP.kernel._ConstRestraint(1, ps)
        rs= IMP.RestraintSet(m, 1.0, "rs")
        rs.add_restraint(r)
        rsf=rs.create_scoring_function()
        self.assertEqual(rsf.evaluate(False), 1)
        r.set_weight(0)
        self.assertEqual(rsf.evaluate(False), 0)
        r.set_weight(1)
        rs.set_weight(0)
        self.assertEqual(rsf.evaluate(False), 0)
if __name__ == '__main__':
    IMP.test.main()
