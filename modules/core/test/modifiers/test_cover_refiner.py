import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import random

class TestREFCover(IMP.test.TestCase):
    """Tests for RefinerCover"""

    def check_cover(self, p, ps):
        d= IMP.core.XYZRDecorator(p)
        d.show()
        for cp in ps:
            dc= IMP.core.XYZRDecorator(cp)
            dc.show()
            #d.get_sphere().get_center().show()
            #dc.get_sphere().get_center().show()
            self.assert_(d.get_sphere().get_contains(dc.get_sphere()))

    def test_it(self):
        """Test cover refined"""
        m= IMP.Model()
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
        #c= IMP.core.CentroidOfRefined(r)
        c= IMP.core.CoverRefined(r, IMP.core.XYZRDecorator.get_default_radius_key(), 1)
        c.apply(pp)
        self.check_cover(pp, ps)

    def test_it(self):
        """Test cover refined helper"""
        m= IMP.Model()
        IMP.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=IMP.core.create_xyzr_particles(m, 10, 1)
        p=IMP.Particle(m)
        ss= IMP.core.create_cover(p,
                                 IMP.core.FixedParticleRefiner(ps),
                                 IMP.core.XYZRDecorator.get_default_radius_key(), 1)
        sss= m.get_score_states()
        for s in sss:
            s.show()
        print "eval"
        m.evaluate(True)
        self.check_cover(p, ps)
if __name__ == '__main__':
    unittest.main()
