import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle

from math import *


class NormalMoverTest(IMP.test.TestCase):

    mv = IMP.core.NormalMover

    def _make_stuff(self):
        IMP.test.TestCase.setUp(self)
        m = IMP.Model()
        pa = m.add_particle("p")
        att = IMP.FloatKey("test")
        m.add_attribute(att, pa, 5.0)
        m.set_is_optimized(att, pa, True)
        mv = self.mv(m, pa, [att], 1.0)
        return m, mv, pa, att

    def test_propose(self):
        m, mv, pa, att = self._make_stuff()
        old = m.get_attribute(att, pa)
        result = mv.propose()
        new = m.get_attribute(att, pa)
        parts = result.get_moved_particles()
        self.assertEqual(len(parts), 1)
        self.assertEqual(parts[0], pa)
        self.assertTrue(abs(old - new) > 1e-7)

    def test_reject(self):
        m, mv, pa, att = self._make_stuff()
        old = m.get_attribute(att, pa)
        mv.propose()
        mv.reject()
        new = m.get_attribute(att, pa)
        self.assertAlmostEqual(new, old)

    def test_pickle(self):
        m, mv, pa, att = self._make_stuff()
        mv.set_name("foo")
        dump = pickle.dumps(mv)

        newmv = pickle.loads(dump)
        self.assertEqual(newmv.get_name(), "foo")

    def test_pickle_polymorphic(self):
        m, mv, pa, att = self._make_stuff()
        mv.set_name("foo")
        sm = IMP.core.SerialMover([mv])
        dump = pickle.dumps(sm)

        newsm = pickle.loads(dump)
        newmv, = newsm.get_movers()
        self.assertEqual(newmv.get_name(), "foo")


class LogNormalMoverTest(NormalMoverTest):

    mv = IMP.core.LogNormalMover


if __name__ == '__main__':
    IMP.test.main()
