import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random

tk= IMP.IntKey("type")
class Pred(IMP.PairPredicate):
    def __init__(self):
        IMP.PairPredicate.__init__(self)
    def get_value(self, pp):
        return pp[0].get_value(tk)+pp[1].get_value(tk)
    def _do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]
class Score(IMP.PairScore):
    def __init__(self, v):
        self._value=v
        self._pred=Pred()
        self._pred.set_was_used(True)
        IMP.PairScore.__init__(self)
    def evaluate(self, pp, da):
        if self._pred.get_value(pp)==self._value:
            return 0
        else:
            print "uh oh", pp[0].get_value(tk), pp[1].get_value(tk), self._value
            return 1
    def _do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]
class Tests(IMP.test.TestCase):
    def test_it(self):
        """Test PredicatePairsRestraints"""
        m=IMP.Model()
        IMP.base.set_log_level(IMP.base.VERBOSE)
        ps= self.create_particles_in_box(m, 20)
        for i, p in enumerate(ps):
            p.add_attribute(tk, i%5)
        print 1
        threshold=1
        cpss= IMP.container.ClosePairContainer(ps, 1,
                                          IMP.core.QuadraticClosePairsFinder(),
                                          1)
        cpss.set_was_used(True)
        pred=Pred()
        r= IMP.container.PredicatePairsRestraint(pred, cpss)
        for i in range(1,9):
            r.set_score(i, Score(i))
        r.set_unknown_score(Score(0))
        for p in ps:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        # move things a little
        self.assertEqual(r.evaluate(False), 0)
        for i in range(0,10):
            for p in ps:
                d= IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates()
                                  + IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 5)))
            self.assertEqual(r.evaluate(False), 0)
if __name__ == '__main__':
    IMP.test.main()
