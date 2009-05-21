import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.helper
import random

class TestREFCover(IMP.test.TestCase):
    """Tests for RefinerCover"""

    def test_it(self):
        """Test centroid of refined helper"""
        m= IMP.Model()
        IMP.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=IMP.core.create_xyzr_particles(m, 10, 1)
        fpr= IMP.core.FixedRefiner(ps)
        p= IMP.helper.create_centroid(IMP.Particle(m), fpr)
        m.add_score_state(p)
        ss= m.get_score_states()
        for s in ss:
            s.show()
        print "eval"
        m.evaluate(True)
if __name__ == '__main__':
    unittest.main()
