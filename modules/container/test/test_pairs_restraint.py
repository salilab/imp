import IMP
import IMP.test
import IMP.container


class Tests(IMP.test.TestCase):

    """Test PairsRestraint"""

    def test_accessors(self):
        """Test accessors of PairsRestraint"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        lpc = IMP.container.ListPairContainer(m, [(p1, p2)])
        p1 = IMP._ConstPairScore(10.0)
        r1 = IMP.container.PairsRestraint(p1, lpc)

        s = r1.get_score_object()
        self.assertIsInstance(s, IMP.PairScore)
        c = r1.get_container()
        self.assertIsInstance(c, IMP.PairContainer)


if __name__ == '__main__':
    IMP.test.main()
