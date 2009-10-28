import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import random

class TestBL(IMP.test.TestCase):
    def _are_close(self, a, b, d):
        da= IMP.core.XYZR(a)
        db= IMP.core.XYZR(b)
        cd= IMP.core.distance(da, db)
        return (cd <= d)

    def _compare_lists(self, m, pc, d, out):
        print "comparing"
        print m.get_number_of_score_states()
        m.evaluate(True)
        print "list is " + str(out.get_number_of_particle_pairs())
        for p in out.get_particle_pairs():
            print p
        for i in range(0, pc.get_number_of_particles()):
            for j in range(0, i):
                a= pc.get_particle(i)
                b= pc.get_particle(j)
                pp= IMP.ParticlePair(a,b)
                if self._are_close(a,b, d):
                    print "expecting "+str(pp)
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(a,b))
                                 or out.get_contains_particle_pair(IMP.ParticlePair(b,a)))
    #DO NOT COMMMIT
    def _test_it(self):
        """Test ClosePairContainer"""
        m=IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        ps= self.create_particles_in_box(m, 20)
        # test rebuilding under move, set input and change radius
        pc= IMP.core.ListSingletonContainer(ps)
        print "creat cpss "+str(pc)
        #IMP.set_log_level(IMP.VERBOSE)
        print 1
        threshold=1
        cpss= IMP.core.ClosePairContainer(pc, threshold,
                                          IMP.core.QuadraticClosePairsFinder(),
                                          1)
        for p in ps:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        self._compare_lists(m, pc, threshold, cpss)

        # move things a little
        for p in ps:
            d= IMP.core.XYZ(p)
            d.set_coordinates(d.get_coordinates()
                              + IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(0,0,0), .55))
        print "first time"
        self._compare_lists(m, pc, threshold, cpss)
        # move things a lot
        for i in range(0,10):
            print "moving"
            for p in ps:
                d= IMP.core.XYZ(p)
                d.set_coordinates(d.get_coordinates()
                                  + IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(0,0,0), .7*(i+1)))
            self._compare_lists(m, pc, threshold, cpss)


    def test_restraint(self):
        """Test ClosePairContainer with a restraint"""
        m=IMP.Model()
        IMP.set_log_level(IMP.VERBOSE)
        ps= self.create_particles_in_box(m, 10)
        # test rebuilding under move, set input and change radius
        pc= IMP.core.ListSingletonContainer(ps)
        print "creat cpss "+str(pc)
        #IMP.set_log_level(IMP.VERBOSE)
        print 1
        threshold=1
        cpss= IMP.core.ClosePairContainer(pc, threshold,
                                          IMP.core.QuadraticClosePairsFinder(),
                                          1)
        r= IMP.core.PairsRestraint(IMP.core.DistancePairScore(IMP.core.Harmonic(3, 1)), cpss)
        m.set_is_incremental(True)
        m.add_restraint(r)
        for p in ps:
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(random.uniform(0,2))
        self._compare_lists(m, pc, threshold, cpss)

        # move things a little
        for p in ps:
            d= IMP.core.XYZ(p)
            d.set_coordinates(d.get_coordinates()
                              + IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(0,0,0), .55))
        print "first time"
        self._compare_lists(m, pc, threshold, cpss)
        # move things a lot
        for i in range(0,10):
            print "moving"
            j=0
            for p in ps:
                j=j+1
                if ((i+j)%2) == 0:
                    print "Moving particle " +str(p.get_name())
                    d= IMP.core.XYZ(p)
                    d.set_coordinates(d.get_coordinates()
                                      + IMP.algebra.random_vector_in_sphere(IMP.algebra.Vector3D(0,0,0), .7*(i+1)))
            self._compare_lists(m, pc, threshold, cpss)

if __name__ == '__main__':
    unittest.main()
