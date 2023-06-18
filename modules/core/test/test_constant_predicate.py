from __future__ import print_function
import IMP
import IMP.container
import IMP.core
import IMP.test
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of ConstantSingletonPredicate"""
        m = IMP.Model()
        p = IMP.Particle(m)
        pred = IMP.core.ConstantSingletonPredicate(42, "foo")
        self.assertEqual(pred.get_value_index(m, p), 42)
        dump = pickle.dumps(pred)

        newpred = pickle.loads(dump)
        self.assertEqual(newpred.get_name(), "foo")
        self.assertEqual(newpred.get_value_index(m, p), 42)

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of ConstantSingletonPredicate via polymorphic ptr"""
        m = IMP.Model()
        p = IMP.Particle(m)
        pred = IMP.core.ConstantSingletonPredicate(42, "foo")
        lsc = IMP.container.ListSingletonContainer(m, [p])
        cont = IMP.container.PredicateSingletonsRestraint(pred, lsc)
        cont.set_score(42, IMP._ConstSingletonScore(10.0))

        dump = pickle.dumps(cont)
        newcont = pickle.loads(dump)
        self.assertAlmostEqual(newcont.evaluate(False), 10.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
