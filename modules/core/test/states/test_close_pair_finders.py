import unittest
import IMP, IMP.test
import IMP.core
import os.path
import random

class TestCPFL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def get_distance(self, rk, p0, p1):
        r0=0
        r1=0
        if p0.has_attribute(rk): r0= p0.get_value(rk)
        if p1.has_attribute(rk): r1= p1.get_value(rk)
        d= IMP.core.distance(IMP.core.XYZDecorator(p0),
                             IMP.core.XYZDecorator(p1))
        return d-r0-r1

    def do_test_one(self, cpf):
        IMP.set_log_level(IMP.TERSE)
        dist= random.uniform(0,2)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        rk= IMP.FloatKey("boo")
        #cpf.set_radius_key(rk)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, 200)
        for i in range(0, len(ps)):
            ps[i].add_attribute(rk, random.uniform(0,1))
        pc= IMP.core.ListSingletonContainer(ps)
        pc.show()
        out= IMP.core.FilteredListPairContainer()
        out.show()
        cpf.add_close_pairs(pc, dist, rk, out)
        print "done " + str(out.get_number_of_particle_pairs())
        out.show()
        pc.show()
        for i in range(0, len(ps)):
            for j in range(0,i):
                d= self.get_distance(rk, ps[i], ps[j])
                #d=1000
                if d <= dist:
                    #print "searching for "+str(ps[i].get_name()) + " "\
                    #    + str(ps[j].get_name())
                    #XYZDecorator(ps[
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps[j]))
                                 or
                                 out.get_contains_particle_pair(IMP.ParticlePair(ps[j],
                                                                                 ps[i])))
        print "Done with all test "+str(out.get_number_of_particle_pairs())
        ps2= self.create_particles_in_box(m, 200)
        for i in range(0, len(ps2)):
            ps2[i].add_attribute(rk, random.uniform(0,2))
        pc2= IMP.core.ListSingletonContainer(ps2)
        out= IMP.core.FilteredListPairContainer()

        cpf.add_close_pairs(pc, pc2, dist, rk, out)
        print "done bipartite " + str(out.get_number_of_particle_pairs())
        for i in range(0, len(ps)):
            for j in range(0,len(ps2)):
                #print "testing " + str(ps[i].get_name()) + " " + str(ps2[j].get_name())
                d= self.get_distance(rk, ps[i], ps2[j])
                #print d
                if d <= dist:
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps2[j])))
        print "done with bipartite test"
    def test_quadratic(self):
        """Testing QuadraticClosePairsFinder"""
        print "quadratic"
        self.do_test_one(IMP.core.QuadraticClosePairsFinder())
    def test_sweep(self):
        """Testing BoxSweepClosePairsFinder"""
        print "have cgal"
        self.do_test_one(IMP.core.BoxSweepClosePairsFinder())
    def test_grid(self):
        """Testing GridClosePairsFinder"""
        print "grid"
        self.do_test_one(IMP.core.GridClosePairsFinder())


if __name__ == '__main__':
    unittest.main()
