import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random
import pickle

tk = IMP.IntKey("type")


class Pred(IMP.PairPredicate):

    def __init__(self):
        IMP.PairPredicate.__init__(self)

    def get_value_index(self, m, pp):
        return m.get_attribute(tk, pp[0]) + m.get_attribute(tk, pp[1])

    def do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]


class Score(IMP.PairScore):

    def __init__(self, v):
        self._value = v
        self._pred = Pred()
        self._pred.set_was_used(True)
        IMP.PairScore.__init__(self)

    def evaluate_index(self, m, pp, da):
        if self._pred.get_value_index(m, pp) == self._value:
            return 0
        else:
            print("uh oh", m.get_attribute(tk, pp[0]),
                  m.get_attribute(tk, pp[1]), self._value)
            return 1

    def do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]


class Tests(IMP.test.TestCase):

    def test_it(self):
        """Test PredicatePairsRestraints"""
        m = IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        ps = self.create_particles_in_box(m, 20)
        for i, p in enumerate(ps):
            p.add_attribute(tk, i % 5)
        print(1)
        threshold = 1
        cpss = IMP.container.ClosePairContainer(ps, 1,
                                                IMP.core.QuadraticClosePairsFinder(
                                                ),
                                                1)
        cpss.set_was_used(True)
        pred = Pred()
        r = IMP.container.PredicatePairsRestraint(pred, cpss)
        for i in range(1, 9):
            r.set_score(i, Score(i))
        r.set_unknown_score(Score(0))
        for p in ps:
            d = IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0, 2))
        # move things a little
        self.assertEqual(r.evaluate(False), 0)
        for i in range(0, 10):
            for p in ps:
                d = IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates()
                                  + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0, 0, 0), 5)))
            self.assertEqual(r.evaluate(False), 0)

    def make_system(self):
        m = IMP.Model()
        ps = self.create_particles_in_box(m, 20)
        lsc = IMP.container.ListSingletonContainer(m, ps)
        pred = IMP.core.ConstantSingletonPredicate(42)
        r = IMP.container.PredicateSingletonsRestraint(pred, lsc)
        return m, r

    def test_unknown_score(self):
        """Test PredicateSingletonsRestraint handling of unknown score"""
        m, r = self.make_system()
        # By default, an error if a particle doesn't have a score set
        self.assertRaisesUsageException(r.evaluate, False)
        r.set_is_complete(False)
        self.assertAlmostEqual(r.evaluate(False), 0.0, delta=0.1)
        # 20 particles * 10 = 200
        r.set_unknown_score(IMP._ConstSingletonScore(10))
        self.assertAlmostEqual(r.evaluate(False), 200.0, delta=0.1)

    def test_set_score(self):
        """Test PredicateSingletonsRestraint handling of set score"""
        m, r = self.make_system()
        r.set_score(42, IMP._ConstSingletonScore(10))
        # 20 particles * 10 = 200
        self.assertAlmostEqual(r.evaluate(False), 200.0, delta=0.1)

    def test_pickle(self):
        """Test (un-pickle) of PredicateSingletonsRestraint"""
        m, r = self.make_system()
        r.set_score(42, IMP._ConstSingletonScore(10))
        r.set_name("foo")
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        # 20 particles * 10 = 200
        self.assertAlmostEqual(newr.evaluate(False), 200.0, delta=0.1)

    def test_pickle_polymorphic(self):
        """Test (un-pickle) of PredicateSingletonsRestraint via poly ptr"""
        m, r = self.make_system()
        r.set_score(42, IMP._ConstSingletonScore(10))
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        dump = pickle.dumps(sf)

        newsf = pickle.loads(dump)
        newr, = newsf.restraints

        self.assertEqual(newr.get_name(), "foo")
        # 20 particles * 10 = 200
        self.assertAlmostEqual(newr.evaluate(False), 200.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
