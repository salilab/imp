import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import random

class TestBL(IMP.test.TestCase):
    def _are_close(self, a, b, rk, d):
        da= IMP.core.XYZ(a)
        db= IMP.core.XYZ(b)
        r=0
        if rk != IMP.FloatKey():
            r= a.get_value(rk)+ b.get_value(rk)
        cd= IMP.core.distance(da, db)
        return (cd - r <= d)

    def _compare_lists(self, m, pc0, pc1, out, d):
        print "comparing"
        m.evaluate(False)
        print "list is " + str(out.get_number_of_particle_pairs())
        for a in pc0:
            for b in pc1:
                if self._are_close(a,b, IMP.core.XYZR.get_default_radius_key(), d):
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(a,b)))

    def test_it(self):
        """Test CloseBipartitePairContainer"""
        m=IMP.Model()
        ps0= self.create_particles_in_box(m, 30)
        ps1= self.create_particles_in_box(m, 30)
        # test rebuilding under move, set input and change radius
        pc0= IMP.container.ListSingletonContainer(ps0)
        pc1= IMP.container.ListSingletonContainer(ps1)
        print "creat cpss "+str(pc0)
        #IMP.set_log_level(IMP.VERBOSE)
        print 1
        threshold=.3
        cpss= IMP.container.CloseBipartitePairContainer(pc0, pc1,threshold,3 )

        print "adding a radius"
        for p in ps0:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        for p in ps1:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        self._compare_lists(m, ps0, ps1, cpss, threshold)

        for p in ps0:
            d= IMP.core.XYZR.decorate_particle(p)
            d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
        self._compare_lists(m, ps0, ps1, cpss, threshold)

        for p in ps1:
            d= IMP.core.XYZR.decorate_particle(p)
            d.set_coordinates(IMP.algebra.random_vector_in_unit_box())
        self._compare_lists(m, ps0, ps1, cpss, threshold)





if __name__ == '__main__':
    unittest.main()
