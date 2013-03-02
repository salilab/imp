import IMP
import IMP.test
import IMP.core
import IMP.algebra
import random

class Tests(IMP.test.TestCase):
    """Tests for RefinerCover"""
    def test_it(self):
        """Test cover refined"""
        m= IMP.Model()
        IMP.base.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=[]
        ds=IMP.core.XYZRs()
        pp= IMP.Particle(m)
        hd= IMP.core.Hierarchy.setup_particle(pp)
        dd= IMP.core.XYZR.setup_particle(pp)
        for i in range(0,n):
            p= IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            ps.append(p)
            ds.append(d)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
            d.set_radius(random.uniform(0,1))
            hd.add_child(IMP.core.Hierarchy.setup_particle(p))
        r= IMP.core.ChildrenRefiner(IMP.core.Hierarchy.get_default_traits())
        c= IMP.core.CentroidOfRefined(r)
        c.apply(pp)
        bb= IMP.algebra.BoundingBox3D()
        cd= IMP.core.XYZ(pp)
        for i in range(0,n):
            d= IMP.core.XYZ(hd.get_child(i).get_particle())
            bb+=IMP.algebra.BoundingBox3D(d.get_coordinates())
        self.assertTrue(bb.get_contains(cd.get_coordinates()))
if __name__ == '__main__':
    IMP.test.main()
