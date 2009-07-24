import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra
import random

class TestREFCover(IMP.test.TestCase):
    """Tests for CoverBond"""

    def check_cover(self, p, ps):
        d= IMP.core.XYZR(p)
        d.show()
        s=d.get_sphere()
        cs=IMP.algebra.Sphere3D(s.get_center(),
                                s.get_radius()*1.1)
        for cp in ps:
            dc= IMP.core.XYZR(cp)
            dc.show()
            #d.get_sphere().get_center().show()
            #dc.get_sphere().get_center().show()
            self.assert_(cs.get_contains(dc.get_sphere()))

    def test_it(self):
        """Test cover bond"""
        m= IMP.Model()
        ps=IMP.Particles()
        ds=IMP.core.XYZRs()
        for i in range(0,2):
            p= IMP.Particle(m)
            d=IMP.core.XYZR.create(p)
            ps.append(p)
            ds.append(d)
            d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
            d.set_radius(0)
        #c= IMP.core.CentroidOfRefined(r)
        b= IMP.atom.custom_bond(IMP.atom.Bonded.create(ps[0]),
                                IMP.atom.Bonded.create(ps[1]),
                                5, 1)
        IMP.core.XYZR.create(b.get_particle())
        c= IMP.atom.CoverBond()
        c.apply(b.get_particle())
        self.check_cover(b.get_particle(), ps)

if __name__ == '__main__':
    unittest.main()
