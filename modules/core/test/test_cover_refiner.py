import IMP
import IMP.test
import IMP.core
import IMP.algebra
import random

class Tests(IMP.test.TestCase):
    """Tests for RefinerCover"""

    def check_cover(self, p, ps):
        d= IMP.core.XYZR(p)
        s= IMP.algebra.Sphere3D(d.get_coordinates(),
                                d.get_radius()*1.1)
        d.show()
        for cp in ps:
            dc= IMP.core.XYZR(cp.get_particle())
            dc.show()
            #d.get_sphere().get_center().show()
            #dc.get_sphere().get_center().show()
            self.assertTrue(s.get_contains(dc.get_sphere()))
    def test_it(self):
        """Test cover refined decorator"""
        m= IMP.Model()
        IMP.base.set_log_level(IMP.MEMORY)
        n= random.randrange(1,10)
        ps=IMP.core.create_xyzr_particles(m, 10, 1)
        p=IMP.Particle(m)
        ss= IMP.core.Cover.setup_particle(p,
                                          IMP.core.FixedRefiner(ps))
        m.update()
        self.check_cover(p, ps)
if __name__ == '__main__':
    IMP.test.main()
