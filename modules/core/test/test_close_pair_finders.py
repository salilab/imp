import IMP
import IMP.test
import IMP.core
import IMP.container
import sys
import random

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.TERSE)

    def get_distance(self, p0, p1):
        d0=IMP.core.XYZR(p0)
        d1=IMP.core.XYZR(p1)
        d= IMP.core.get_distance(d0, d1)
        #print str(d0.get_x()) + " " + str(d0.get_y()) + " " + str(d0.get_z()) + " " + str(r0)
        #print str(d1.get_x()) + " " + str(d1.get_y()) + " " + str(d1.get_z()) + " " + str(r1)
        return d
    def do_test_points(self, cpf, num, rmin, rmax):
        #IMP.base.set_log_level(IMP.base.SILENT)
        dist= random.uniform(rmin,rmax)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        #cpf.set_radius_key(rk)
        cpf.set_distance(dist)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, num)
        for i in range(0, len(ps)):
            IMP.core.XYZR.setup_particle(ps[i], random.uniform(rmin, rmax))
        #out.show()
        print "searching"
        cps= cpf.get_close_pairs(ps)
        print "done "
        self._check_close_pairs(ps, dist, cps)
        print "Done with all test"
    def do_test_bi_points(self, cpf, num, rmin, rmax):
        #IMP.base.set_log_level(IMP.base.SILENT)
        dist= random.uniform(0,2)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        #cpf.set_radius_key(rk)
        cpf.set_distance(dist)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, num)
        for i in range(0, len(ps)):
            IMP.core.XYZR.setup_particle(ps[i], random.uniform(rmin, rmax))
        psp= self.create_particles_in_box(m, num)
        for i in range(0, len(psp)):
            IMP.core.XYZR.setup_particle(psp[i], random.uniform(rmin, rmax))
        cps= cpf.get_close_pairs(ps, psp)
        print "done "
        self._check_biclose_pairs(ps, psp,dist, cps)
        print "Done with all test"


    def do_test_one(self, cpf):
        #cpf.set_log_level(IMP.base.SILENT);
        self.do_test_bi_points(cpf, 100, .01,1)
        #cpf.set_log_level(IMP.base.SILENT);
        self.do_test_points(cpf, 100, 1,1)


    def _check_close_pairs(self, ps, dist, found):
        print "testing results with "
        print " ".join([str((x[0].get_name(), x[1].get_name())) for x in found])
        for f in found:
            self.assertNotIn((f[1], f[0]), found)
            self.assertNotEqual(f[0], f[1])
        for i in range(0, len(ps)):
            for j in range(0,i):
                d= self.get_distance(ps[i], ps[j])
                #d=1000
                if d <= .99*dist:
                    cond0=(ps[i],ps[j]) in found
                    cond1=(ps[j],ps[i]) in found
                    self.assertTrue(cond0 or cond1)

    def _check_biclose_pairs(self, ps, ps2, dist, out):
        for i in range(0, len(ps)):
            for j in range(0,len(ps2)):
                if ps[i]== ps2[j]: continue
                #print "testing " + str(ps[i].get_name()) + " " + str(ps2[j].get_name())
                d= self.get_distance(ps[i], ps2[j])
                #print d
                if d <= .95*dist:
                    self.assertTrue((ps[i],
                                                     ps2[j]) in out,
                                    "Pair " +str(ps[i]) + " " +str(ps2[j])
                                    + " not found " + str(d) + " " + str(dist))
        print "done with bipartite test"


    def _check_abiclose_pairs(self, ps, ps2, dist, out):

        for i in range(0, len(ps)):
            for j in range(0,len(ps2)):
                if ps[i]== ps2[j]: continue
                #print "testing " + str(ps[i].get_name()) + " " + str(ps2[j].get_name())
                d= self.get_distance(ps[i], ps2[j])
                #print d
                if d <= .95*dist:
                    self.assertTrue(((ps[i],
                                                      ps2[j]) in out)
                                    or ((ps2[j],
                                                         ps[i]) in out),
                                    "Pair " +str(ps[i].get_name()) + " " +ps2[j].get_name()
                                    + " not found " + str(d) + " " + str(dist))
                    print "found pair " +str(ps[i].get_name()) + " " + str(ps2[j].get_name())
        print "done with bipartite test"
    def test_quadratic(self):
        """Testing QuadraticClosePairsFinder"""
        print "quadratic"
        self.do_test_one(IMP.core.QuadraticClosePairsFinder())
    def test_nn(self):
        """Testing NearestNeighborsClosePairsFinder"""
        print "quadratic"
        self.do_test_one(IMP.core.NearestNeighborsClosePairsFinder())

    def test_sweep(self):
        """Testing BoxSweepClosePairsFinder"""
        if not hasattr(IMP.core, 'BoxSweepClosePairsFinder'):
            self.skipTest("no CGAL support")
        self.do_test_one(IMP.core.BoxSweepClosePairsFinder())

    def test_grid(self):
        """Testing GridClosePairsFinder"""
        print "grid"
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        self.do_test_one(IMP.core.GridClosePairsFinder())

    def test_rigid(self):
        "Testing RigidClosePairsFinder"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        IMP.random_number_generator.seed(1)
        dist= random.uniform(0,2)
        nump=100
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        #cpf.set_radius_key(rk)
        m=IMP.Model()
        ps= IMP.core.create_xyzr_particles(m, nump, 1)
        rbpsa= []
        rbpsb= []
        fps= []
        free_ps=[]
        for i in range(0, len(ps)):
            if i%3==0:
                rbpsa.append(ps[i])
                ps[i].set_name("rbpa"+str(i))
            elif i%3==1:
                rbpsb.append(ps[i])
                ps[i].set_name("rbpb"+str(i))
            else:
                fps.append(ps[i])
                free_ps.append(ps[i])
        rba= IMP.core.RigidBody.setup_particle(IMP.Particle(m), rbpsa)
        rbb= IMP.core.RigidBody.setup_particle(IMP.Particle(m), rbpsb)
        rba.get_particle().set_name("rba")
        rbb.get_particle().set_name("rbb")
        print "before", [IMP.core.XYZR(p) for p in rbpsa]
        m.update()
        print "after", [IMP.core.XYZR(p) for p in rbpsa]
        fps=fps+rba.get_members()
        fps=fps+rbb.get_members()
        cpf= IMP.core.RigidClosePairsFinder()
        cpf.set_distance(dist)
        cps=cpf.get_close_pairs(fps)
        self._check_abiclose_pairs(free_ps, free_ps, dist, cps)
        self._check_abiclose_pairs(free_ps, rbpsa, dist, cps)
        self._check_abiclose_pairs(free_ps, rbpsb, dist, cps)
        self._check_abiclose_pairs(rbpsa, rbpsb, dist, cps)
        print "Done with all test "+str(len(cps))
        ps2= self.create_particles_in_box(m, nump)
        rbpsa2= []
        rbpsb2= []
        fps2= []
        s0= IMP.algebra.Sphere1D(IMP.algebra.Vector1D(.5), .5)
        for i in range(0, len(ps2)):
            ps2[i].add_attribute(IMP.core.XYZR.get_radius_key(),\
                                 IMP.algebra.get_random_vector_in(s0)[0])
            if i%3==0:
                rbpsa2.append(ps2[i])
            elif i%3==1:
                rbpsb2.append(ps2[i])
            else:
                fps2.append(ps2[i])
        rba2= IMP.core.RigidBody.setup_particle(IMP.Particle(m), IMP.core.XYZs(rbpsa2))
        rbb2= IMP.core.RigidBody.setup_particle(IMP.Particle(m), IMP.core.XYZs(rbpsb2))
        fps.append(rba2.get_particle())
        fps.append(rbb2.get_particle())
        rba2.get_particle().set_name("rba2")
        rbb2.get_particle().set_name("rbb2")

        cps=cpf.get_close_pairs(fps, ps2)
        print "done bipartite " + str(len(cps))
        print ps
        self._check_biclose_pairs(ps, ps2, dist,cps)

if __name__ == '__main__':
    IMP.test.main()
