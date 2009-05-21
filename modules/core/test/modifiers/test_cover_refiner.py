import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
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
        """Test cover refined"""
        m= IMP.Model()
        n= random.randrange(1,10)
        ps=IMP.Particles()
        ds=IMP.core.XYZRs()
        pp= IMP.Particle(m)
        hd= IMP.core.Hierarchy.create(pp)
        dd= IMP.core.XYZR.create(pp)
        for i in range(0,n):
            p= IMP.Particle(m)
            d=IMP.core.XYZR.create(p)
            ps.append(p)
            ds.append(d)
            d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
            d.set_radius(random.uniform(0,1))
            hd.add_child(IMP.core.Hierarchy.create(p))
        r= IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())
        #c= IMP.core.CentroidOfRefined(r)
        c= IMP.core.CoverRefined(r, IMP.core.XYZR.get_default_radius_key(), 1)
        c.apply(pp)
        self.check_cover(pp, ps)

if __name__ == '__main__':
    unittest.main()
