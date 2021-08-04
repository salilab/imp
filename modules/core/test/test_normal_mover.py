import IMP
import IMP.test
import IMP.core
import IMP.container

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
        if IMP.IMP_KERNEL_HAS_NUMPY:
            parts = [IMP.ParticleIndex(p) for p in parts]
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


class LogNormalMoverTest(NormalMoverTest):

    mv = IMP.core.LogNormalMover


if __name__ == '__main__':
    IMP.test.main()
