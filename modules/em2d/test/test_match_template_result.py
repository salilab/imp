import IMP
import IMP.test
import IMP.em2d
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of MatchTemplateResult"""
        r = IMP.em2d.MatchTemplateResult([1, 2], 42.0)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.pair, [1, 2])
        self.assertAlmostEqual(newr.cross_correlation, 42.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
