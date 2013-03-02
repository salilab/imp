import IMP
import IMP.test
import IMP.core
import math


class Tests(IMP.test.TestCase):
    """Tests for all pairs pair container"""

    def _test_allp(self):
        """Check that removal from a list container works"""
        m= IMP.Model()
        ps= []
        psr= []
        for i in range(0,50):
            p= IMP.Particle(m)
            ps.append(p)
            if i%5== 0:
                psr.append(p)
        for p in psr:
            ps.remove(p)
        for p0 in lp.get_particles():
            for pr in psr:
                self.assertNotEqual(p0, pr)
        print "bye0"

    def test_allp2(self):
        """Check that removal from a list container works and removal from model"""
        m= IMP.Model()
        ps= []
        psr= []
        for i in range(0,50):
            p= IMP.Particle(m)
            ps.append(p)
            if i%5== 0:
                psr.append(p)
        for p in psr:
            ps.remove(p)
        for p in psr:
            m.remove_particle(p)
        print "bye"
        # make sure cleanup goes without error



if __name__ == '__main__':
    IMP.test.main()
