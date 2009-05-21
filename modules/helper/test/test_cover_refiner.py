import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.helper
import random

class TestREFCover(IMP.test.TestCase):
    """Tests for RefinerCover"""

    def check_cover(self, p, ps):
        d= IMP.core.XYZR(p)
        d.show()
        for cp in ps:
            dc= IMP.core.XYZR(cp)
            dc.show()
            #d.get_sphere().get_center().show()
            #dc.get_sphere().get_center().show()
            self.assert_(d.get_sphere().get_contains(dc.get_sphere()))
    def test_it(self):
        """Test cover refined helper"""
        m= IMP.Model()
        IMP.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=IMP.core.create_xyzr_particles(m, 10, 1)
        p=IMP.Particle(m)
        ss= IMP.helper.create_cover(p,
                                 IMP.core.FixedRefiner(ps),
                                 IMP.core.XYZR.get_default_radius_key(), 1)
        m.add_score_state(ss)
        sss= m.get_score_states()
        for s in sss:
            s.show()
        print "eval"
        m.evaluate(True)
        self.check_cover(p, ps)
if __name__ == '__main__':
    unittest.main()
