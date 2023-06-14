from __future__ import print_function
import IMP
import IMP.container
import IMP.core
import IMP.test
import pickle

t1 = IMP.core.ParticleType("type1")
t2 = IMP.core.ParticleType("type2")

class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of UnorderedTypeSingletonPredicate"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        IMP.core.Typed.setup_particle(p1, t1)
        p2 = IMP.Particle(m)
        IMP.core.Typed.setup_particle(p2, t2)
        pred = IMP.core.UnorderedTypeSingletonPredicate("foo")
        self.assertEqual(pred.get_value_index(m, p1), t1.get_index())
        self.assertEqual(pred.get_value_index(m, p2), t2.get_index())
        dump = pickle.dumps(pred)

        newpred = pickle.loads(dump)
        self.assertEqual(newpred.get_name(), "foo")
        self.assertEqual(newpred.get_value_index(m, p1), t1.get_index())
        self.assertEqual(newpred.get_value_index(m, p2), t2.get_index())

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of UnorderedTypeSingletonPredicate via poly ptr"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        IMP.core.Typed.setup_particle(p1, t1)
        p2 = IMP.Particle(m)
        IMP.core.Typed.setup_particle(p2, t2)
        pred = IMP.core.UnorderedTypeSingletonPredicate("foo")
        lsc = IMP.container.ListSingletonContainer(m, [p1, p2])
        cont = IMP.container.PredicateSingletonsRestraint(pred, lsc)
        cont.set_score(t1.get_index(), IMP._ConstSingletonScore(10.0))
        cont.set_score(t2.get_index(), IMP._ConstSingletonScore(40.0))

        dump = pickle.dumps(cont)
        newcont = pickle.loads(dump)
        self.assertAlmostEqual(newcont.evaluate(False), 50.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
