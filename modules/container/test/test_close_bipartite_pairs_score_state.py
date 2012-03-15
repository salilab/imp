import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.container
import random

class TestBL(IMP.test.TestCase):
    def _are_close(self, a, b, rk, d):
        da= IMP.core.XYZ(a)
        db= IMP.core.XYZ(b)
        r=0
        if rk != IMP.FloatKey():
            r= a.get_value(rk)+ b.get_value(rk)
        cd= IMP.core.get_distance(da, db)
        return (cd - r <= d)

    def _compare_lists(self, m, pc0, pc1, out, d):
        print "comparing"
        IMP.set_log_level(IMP.VERBOSE)
        m.update()
        print "list is " + str(out.get_number_of_particle_pairs())
        for a in pc0:
            for b in pc1:
                if self._are_close(a,b, IMP.core.XYZR.get_radius_key(), d):
                    self.assertTrue(out.get_contains_particle_pair((a,b)))

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
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        self._compare_lists(m, ps0, ps1, cpss, threshold)

        for p in ps1:
            d= IMP.core.XYZR.decorate_particle(p)
            d.set_coordinates(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
        self._compare_lists(m, ps0, ps1, cpss, threshold)

    def test_filtering(self):
        """Testing that CloseBipartitePairContainer responds to changes in filters"""
        m=IMP.Model()
        ps0= self.create_particles_in_box(m, 30)
        ps1= self.create_particles_in_box(m, 30)
        for p in ps0+ps1:
            IMP.core.XYZR.setup_particle(p, 1)
        # test rebuilding under move, set input and change radius
        pc0= IMP.container.ListSingletonContainer(ps0)
        pc1= IMP.container.ListSingletonContainer(ps1)
        cpss= IMP.container.CloseBipartitePairContainer(pc0, pc1,100000,3 )
        m.update()
        self.assertEqual(cpss.get_number_of_particle_pairs(), len(ps0)*len(ps1))
        abpc= IMP.container.AllBipartitePairContainer(pc0, pc1)
        cpss.add_pair_filter(IMP.container.InContainerPairFilter(abpc))
        print "re-evaluate"
        m.update()
        self.assertEqual(cpss.get_number_of_particle_pairs(), 0)



if __name__ == '__main__':
    IMP.test.main()
