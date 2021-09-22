import IMP
import IMP.test
import IMP.container


class TestMovedScore(IMP.PairScore):
    def __init__(self, value, name="TestMovedScore %1%"):
        IMP.PairScore.__init__(self, name)
        self.value = value

    def evaluate_index(self, m, pis, accum):
        self.moved_pis = None
        self.reset_pis = None
        return self.value

    def do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]


class Tests(IMP.test.TestCase):

    """Test ListPairContainer"""

    def test_scoring(self):
        """Test basic scoring of ListPairContainer"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        lpc = IMP.container.ListPairContainer(m, [(p1, p2)])
        p1 = TestMovedScore(value=42.)
        r1 = IMP.container.PairsRestraint(p1, lpc)
        self.assertAlmostEqual(r1.evaluate(False), 42., delta=1e-6)
        self.assertIsNone(p1.moved_pis)
        self.assertIsNone(p1.reset_pis)

    def test_scoring_moved(self):
        """Test basic moved-scoring of ListPairContainer"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        lpc = IMP.container.ListPairContainer(m, [(p1, p2)])
        p1 = TestMovedScore(value=42.)
        r1 = IMP.container.PairsRestraint(p1, lpc)
        self.assertAlmostEqual(r1.evaluate_moved(False, [], []),
                               42., delta=1e-6)
        self.assertIsNone(p1.moved_pis)
        self.assertIsNone(p1.reset_pis)


if __name__ == '__main__':
    IMP.test.main()
