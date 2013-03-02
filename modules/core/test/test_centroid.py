import IMP
import IMP.test
import IMP.core
import IMP.algebra
import random

class Tests(IMP.test.TestCase):
    """Tests for RefinerCover"""

    def test_it(self):
        """Test centroid of refined decorator"""
        m= IMP.Model()
        IMP.base.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=IMP.core.create_xyzr_particles(m, 10, 1)
        fpr= IMP.core.FixedRefiner(ps)
        p= IMP.core.Centroid.setup_particle(IMP.Particle(m), fpr)
        print "eval"
        m.evaluate(True)
if __name__ == '__main__':
    IMP.test.main()
