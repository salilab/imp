import unittest
import IMP
import IMP.test
import IMP.core
import IMP.helper
import sys
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
        d0=IMP.core.XYZ(p0)
        d1=IMP.core.XYZ(p1)
        d= IMP.core.distance(d0, d1)
        #print str(d0.get_x()) + " " + str(d0.get_y()) + " " + str(d0.get_z()) + " " + str(r0)
        #print str(d1.get_x()) + " " + str(d1.get_y()) + " " + str(d1.get_z()) + " " + str(r1)
        return d-r0-r1

    def do_test_one(self, cpf):
        IMP.set_log_level(IMP.TERSE)
        dist= random.uniform(0,2)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        rk= IMP.FloatKey("boo")
        #cpf.set_radius_key(rk)
        cpf.set_radius_key(rk)
        cpf.set_distance(dist)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, 200)
        for i in range(0, len(ps)):
            ps[i].add_attribute(rk, random.uniform(0,1))
        pc= IMP.core.ListSingletonContainer(ps)
        pc.show()
        out= IMP.core.ListPairContainer()
        out.show()
        cpf.add_close_pairs(pc, out)
        print "done " + str(out.get_number_of_particle_pairs())
        self._check_close_pairs(ps, dist, rk, out)

        print "Done with all test "+str(out.get_number_of_particle_pairs())
        ps2= self.create_particles_in_box(m, 200)
        for i in range(0, len(ps2)):
            ps2[i].add_attribute(rk, random.uniform(0,2))
        pc2= IMP.core.ListSingletonContainer(ps2)
        out= IMP.core.ListPairContainer()

        cpf.add_close_pairs(pc, pc2, out)
        print "done bipartite " + str(out.get_number_of_particle_pairs())
        self._check_biclose_pairs(ps, ps2, dist, rk,out)

    def _check_close_pairs(self, ps, dist, rk, out):
        out.show()
        for i in range(0, len(ps)):
            for j in range(0,i):
                d= self.get_distance(rk, ps[i], ps[j])
                #d=1000
                if d <= .95*dist:
                    #print "searching for "+str(ps[i].get_name()) + " "\
                    #    + str(ps[j].get_name())
                    #XYZ(ps[
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps[j]))
                                 or
                                 out.get_contains_particle_pair(IMP.ParticlePair(ps[j],
                                                                                 ps[i])),
                                 "Pair " +str(ps[i].get_name()) + " " +ps[j].get_name()
                                 + " not found " + str(d) + " " + str(dist))
    def _check_biclose_pairs(self, ps, ps2, dist, rk, out):

        for i in range(0, len(ps)):
            for j in range(0,len(ps2)):
                if ps[i]== ps2[j]: continue
                #print "testing " + str(ps[i].get_name()) + " " + str(ps2[j].get_name())
                d= self.get_distance(rk, ps[i], ps2[j])
                #print d
                if d <= .95*dist:
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps2[j])),
                                  "Pair " +str(ps[i].get_name()) + " " +ps2[j].get_name()
                                 + " not found " + str(d) + " " + str(dist))
        print "done with bipartite test"
    def _check_abiclose_pairs(self, ps, ps2, dist, rk, out):

        for i in range(0, len(ps)):
            for j in range(0,len(ps2)):
                if ps[i]== ps2[j]: continue
                #print "testing " + str(ps[i].get_name()) + " " + str(ps2[j].get_name())
                d= self.get_distance(rk, ps[i], ps2[j])
                #print d
                if d <= .95*dist:
                    self.assert_(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps2[j]))
                                 or out.get_contains_particle_pair(IMP.ParticlePair(ps2[j],
                                                                                 ps[i])),
                                  "Pair " +str(ps[i].get_name()) + " " +ps2[j].get_name()
                                 + " not found " + str(d) + " " + str(dist))
        print "done with bipartite test"

    def test_quadratic(self):
        """Testing QuadraticClosePairsFinder"""
        print "quadratic"
        self.do_test_one(IMP.core.QuadraticClosePairsFinder())
    def test_sweep(self):
        """Testing BoxSweepClosePairsFinder"""
        if hasattr(IMP.core, 'BoxSweepClosePairsFinder'):
            self.do_test_one(IMP.core.BoxSweepClosePairsFinder())
        else:
            sys.stderr.write("Test skipped: no CGAL support: ")
    def test_grid(self):
        """Testing GridClosePairsFinder"""
        print "grid"
        self.do_test_one(IMP.core.GridClosePairsFinder())


    def test_rigid(self):
        "Testing RigidClosePairsFinder"""
        IMP.set_log_level(IMP.VERBOSE)
        dist= random.uniform(0,2)
        nump=100
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        rk= IMP.FloatKey("boo")
        #cpf.set_radius_key(rk)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, nump)
        rbpsa= IMP.Particles()
        rbpsb= IMP.Particles()
        fps= IMP.Particles()
        for i in range(0, len(ps)):
            ps[i].add_attribute(rk, random.uniform(0,1))
            if i%3==0:
                rbpsa.append(ps[i])
                ps[i].set_name("rbpa"+str(i))
            elif i%3==1:
                rbpsb.append(ps[i])
                ps[i].set_name("rbpb"+str(i))
            else:
                fps.append(ps[i])
        rba= IMP.core.RigidBody.create(IMP.Particle(m), IMP.core.XYZs(rbpsa))
        rbb= IMP.core.RigidBody.create(IMP.Particle(m), IMP.core.XYZs(rbpsb))
        rba.get_particle().set_name("rba")
        rbb.get_particle().set_name("rbb")
        IMP.helper.cover_members(rba, rk)
        IMP.helper.cover_members(rbb, rk)
        fps.append(rba.get_particle())
        fps.append(rbb.get_particle())
        pc= IMP.core.ListSingletonContainer(fps)
        pc.show()
        out= IMP.core.ListPairContainer()
        out.show()
        cpf= IMP.core.RigidClosePairsFinder()
        cpf.set_radius_key(rk)
        cpf.set_distance(dist)
        cpf.add_close_pairs(pc, out)
        self._check_abiclose_pairs(fps[0:-2], fps[0:-2], dist, rk, out)
        self._check_abiclose_pairs(fps[0:-2], rbpsa, dist, rk, out)
        self._check_abiclose_pairs(fps[0:-2], rbpsb, dist, rk, out)
        self._check_abiclose_pairs(rbpsa, rbpsb, dist, rk, out)
        print "Done with all test "+str(out.get_number_of_particle_pairs())
        ps2= self.create_particles_in_box(m, nump)
        rbpsa2= IMP.Particles()
        rbpsb2= IMP.Particles()
        fps2= IMP.Particles()
        for i in range(0, len(ps2)):
            ps2[i].add_attribute(rk, random.uniform(0,1))
            if i%3==0:
                rbpsa2.append(ps2[i])
            elif i%3==1:
                rbpsb2.append(ps2[i])
            else:
                fps2.append(ps2[i])
        rba2= IMP.core.RigidBody.create(IMP.Particle(m), IMP.core.XYZs(rbpsa2))
        rbb2= IMP.core.RigidBody.create(IMP.Particle(m), IMP.core.XYZs(rbpsb2))
        IMP.helper.cover_members(rba2, rk)
        IMP.helper.cover_members(rbb2, rk)
        fps.append(rba2.get_particle())
        fps.append(rbb2.get_particle())
        rba2.get_particle().set_name("rba2")
        rbb2.get_particle().set_name("rbb2")

        pc2= IMP.core.ListSingletonContainer(ps2)
        out= IMP.core.ListPairContainer()

        cpf.add_close_pairs(pc, pc2, out)
        print "done bipartite " + str(out.get_number_of_particle_pairs())
        self._check_biclose_pairs(ps, ps2, dist, rk, out)

if __name__ == '__main__':
    unittest.main()
