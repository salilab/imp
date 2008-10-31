import unittest
import IMP, IMP.test
import IMP.core
import os.path
import random

class TestBL(IMP.test.TestCase):
    def _are_close(self, a, b, rk, d):
        da= IMP.core.XYZDecorator(a)
        db= IMP.core.XYZDecorator(b)
        r=0
        if rk != IMP.FloatKey():
            r= a.get_value(rk)+ b.get_value(rk)
        cd= IMP.core.distance(da, db)
        return (cd - r <= d)

    def _compare_lists(self, m, cpss):
        print "comparing"
        m.evaluate(False)
        rk= cpss.get_radius_key()
        d= cpss.get_distance()
        pc0= cpss.get_first_particle_container()
        pc1= cpss.get_second_particle_container()
        out= cpss.get_close_pairs_container()
        print "list is " + str(out.get_number_of_particle_pairs())
        for i in range(0, pc0.get_number_of_particles()):
            for j in range(0, pc1.get_number_of_particles()):
                a= pc0.get_particle(i)
                b= pc1.get_particle(j)
                if self._are_close(a,b, rk, d):
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(a,b)))

    def test_it(self):
        """Test ClosePairsScoreState"""
        m=IMP.Model()
        ps0= self.create_particles_in_box(m, 30)
        ps1= self.create_particles_in_box(m, 30)
        # test rebuilding under move, set input and change radius
        pc0= IMP.core.ListParticleContainer(ps0)
        pc1= IMP.core.ListParticleContainer(ps1)
        print "creat cpss "+str(pc0)
        #IMP.set_log_level(IMP.VERBOSE)
        nrk=IMP.FloatKey()
        print 1
        cpss= IMP.core.CloseBipartitePairsScoreState(pc0, pc1, nrk)
        print 2
        cpss.set_slack(0)
        print 3
        cpss.set_distance(3)
        print "add cpss"
        m.add_score_state(cpss)
        self._compare_lists(m, cpss)

        print "adding a radius"
        cpss.set_distance(0)
        for p in ps0:
            d= IMP.core.XYZRDecorator.create(p)
            d.set_radius(random.uniform(0,2))
        for p in ps1:
            d= IMP.core.XYZRDecorator.create(p)
            d.set_radius(random.uniform(0,2))
        cpss.set_radius_key(IMP.core.XYZRDecorator.get_default_radius_key())
        self._compare_lists(m, cpss)

        for p in ps0:
            d= IMP.core.XYZRDecorator.cast(p)
            d.set_coordinates(IMP.random_vector_in_box())
        self._compare_lists(m, cpss)

        for p in ps1:
            d= IMP.core.XYZRDecorator.cast(p)
            d.set_coordinates(IMP.random_vector_in_box())
        self._compare_lists(m, cpss)

        print "changing radius"
        cpss.set_radius_key(IMP.core.XYZDecorator.get_xyz_keys()[0])
        self._compare_lists(m, cpss)





if __name__ == '__main__':
    unittest.main()
