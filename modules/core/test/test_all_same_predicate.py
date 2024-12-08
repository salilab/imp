import IMP
import IMP.container
import IMP.core
import IMP.test
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of AllSamePairPredicate"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        pred = IMP.core.AllSamePairPredicate("foo")
        self.assertEqual(pred.get_value_index(m, [p1, p2]), 0)
        self.assertEqual(pred.get_value_index(m, [p1, p1]), 1)
        dump = pickle.dumps(pred)

        newpred = pickle.loads(dump)
        self.assertEqual(newpred.get_name(), "foo")
        self.assertEqual(newpred.get_value_index(m, [p1, p2]), 0)
        self.assertEqual(newpred.get_value_index(m, [p1, p1]), 1)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of AllSamePairPredicate via polymorphic ptr"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        pred = IMP.core.AllSamePairPredicate("foo")
        lpc = IMP.container.ListPairContainer(m, [(p1, p1), (p1, p2)])
        cont = IMP.container.PredicatePairsRestraint(pred, lpc)
        cont.set_score(1, IMP._ConstPairScore(10.0))
        cont.set_score(0, IMP._ConstPairScore(40.0))

        dump = pickle.dumps(cont)
        newcont = pickle.loads(dump)
        self.assertAlmostEqual(newcont.evaluate(False), 50.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
