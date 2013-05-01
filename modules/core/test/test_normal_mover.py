import IMP
import IMP.test
import IMP.core
import IMP.container

from math import *

class NormalMoverTest(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        m = IMP.Model()
        pa=IMP.Particle(m)
        att=IMP.FloatKey("test")
        pa.add_attribute(att,5.0)
        mv=IMP.core.NormalMover(m, pa.get_index(), [att], 1.0)
        self.m = m
        self.mv = mv
        self.pa = pa
        self.att = att

    def test_propose(self):
        old=self.pa.get_value(self.att)
        result=self.mv.propose()
        new=self.pa.get_value(self.att)
        parts=result.get_moved_particles()
        self.assertEqual(len(parts), 1)
        self.assertEqual(parts[0], self.pa.get_index())
        self.assertTrue(abs(old-new)>1e-7)

    def test_reject(self):
        old=self.pa.get_value(self.att)
        self.mv.propose()
        self.mv.reject()
        new=self.pa.get_value(self.att)
        self.assertAlmostEqual(new,old)

if __name__ == '__main__':
    IMP.test.main()
