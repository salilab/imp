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
        ds=IMP.core.XYZRDecorators()
        pp= IMP.Particle()
        m.add_particle(pp)
        hd= IMP.core.HierarchyDecorator.create(pp)
        dd= IMP.core.XYZRDecorator.create(pp)
        for i in range(0,n):
            p= IMP.Particle()
            m.add_particle(p)
            d=IMP.core.XYZRDecorator.create(p)
            ps.append(p)
            ds.append(d)
            d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
            d.set_radius(random.uniform(0,1))
            hd.add_child(IMP.core.HierarchyDecorator.create(p))
        r= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        c= IMP.core.CentroidOfRefined(r)
        c.set_slack(.1)
        c.apply(pp)
        for d in ds:
            self.assert_(IMP.core.distance(d, dd) <= 0)
    def test_it(self):
        """Test centroid of refined helper"""
        m= IMP.Model()
        IMP.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=IMP.core.create_xyzr_particles(m, 10, 1)
        p= IMP.core.create_centroid_particle(m, ps);
        ss= m.get_score_states()
        for s in ss:
            s.show()
        print "eval"
        m.evaluate(True)
if __name__ == '__main__':
    unittest.main()
