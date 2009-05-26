import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import random

class TestREFCover(IMP.test.TestCase):
    """Tests for RefinerCover"""
    def test_it(self):
        """Test cover refined"""
        m= IMP.Model()
        IMP.set_log_level(IMP.MEMORY)
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
        c= IMP.core.CentroidOfRefined(r)
        c.apply(pp)
        bb= IMP.algebra.BoundingBox3D()
        cd= IMP.core.XYZ(pp)
        for i in range(0,n):
            d= IMP.core.XYZ(hd.get_child(i).get_particle())
            bb+=IMP.algebra.BoundingBox3D(d.get_coordinates())
        self.assert_(bb.get_contains(cd.get_coordinates()))
if __name__ == '__main__':
    unittest.main()
