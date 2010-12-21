import IMP
import IMP.test
import IMP.core
import IMP.container
import sys
import random

class TestCPFL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def get_distance(self, rk, p0, p1):
        r0=0
        r1=0
        if IMP.get_particle(p0).has_attribute(rk): r0= IMP.get_particle(p0).get_value(rk)
        if IMP.get_particle(p1).has_attribute(rk): r1= IMP.get_particle(p1).get_value(rk)
        d0=IMP.core.XYZ(p0)
        d1=IMP.core.XYZ(p1)
        d= IMP.core.get_distance(d0, d1)
        #print str(d0.get_x()) + " " + str(d0.get_y()) + " " + str(d0.get_z()) + " " + str(r0)
        #print str(d1.get_x()) + " " + str(d1.get_y()) + " " + str(d1.get_z()) + " " + str(r1)
        return d-r0-r1
    def do_test_periodic_points(self, cpf, bb, num, rmin, rmax):
        #IMP.set_log_level(IMP.SILENT)
        dist= random.uniform(rmin,rmax+1)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        rk= IMP.core.XYZR.get_radius_key()
        cpf.set_distance(dist)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, num-2, bb.get_corner(0), bb.get_corner(1))
        ps.append(IMP.core.XYZ.setup_particle(IMP.Particle(m), bb.get_corner(0)))
        ps.append(IMP.core.XYZ.setup_particle(IMP.Particle(m), bb.get_corner(1)))
        for p in ps:
            print IMP.core.XYZ(p).get_coordinates()
        for i in range(0, len(ps)):
            ps[i].add_attribute(rk, random.uniform(rmin, rmax))
        pc= IMP.container.ListSingletonContainer(ps)
        #pc.show()
        out= IMP.container.ListPairContainer(m)
        #out.show()
        print "searching"
        cps= cpf.get_close_pairs(pc)
        print "done " + str(out.get_number_of_particle_pairs())
        print "testing results with "
        print " ".join([str((x[0].get_name(), x[1].get_name())) for x in cps])
        for f in cps:
            self.assertNotIn(IMP.ParticlePair(f[1], f[0]), cps)
            self.assertNotEqual(f[0], f[1])
        sides=[bb.get_corner(1)[i]-bb.get_corner(0)[i] for i in [0,1,2]]
        steps=[[-sides[i], 0, sides[i]] for i in [0,1,2]]
        for i in range(0, len(ps)):
            ri=IMP.core.XYZR(ps[i]).get_radius()
            vi= IMP.core.XYZR(ps[i]).get_coordinates()
            for j in range(0,i):
                close=False
                rj=IMP.core.XYZR(ps[j]).get_radius()
                vj0= IMP.core.XYZR(ps[j]).get_coordinates()
                for sx in steps[0]:
                    for sy in steps[1]:
                        for sz in steps[2]:
                            vj= IMP.algebra.Vector3D(vj0[0]+sx, vj0[1]+sy, vj0[2]+sz)
                            if (vj-vi).get_magnitude() < dist + ri+rj*1.01:
                                close=True
                #d=1000
                if close:
                    #print "searching for "+str(ps[i].get_name()) + " "\
                    #    + str(ps[j].get_name())
                    #XYZ(ps[
                    self.assertTrue(IMP.ParticlePair(ps[i],ps[j]) in cps
                                 or
                                 IMP.ParticlePair(ps[j],ps[i]) in cps,
                                 "Pair " +str(ps[i].get_name()) + " " +ps[j].get_name()
                                 + " not found "+str(vi)+" " +str(vj0))
                    print "found pair " +str(ps[i].get_name()) + " " + str(ps[j].get_name())
    def do_test_points(self, cpf, num, rmin, rmax):
        #IMP.set_log_level(IMP.SILENT)
        dist= random.uniform(rmin,rmax)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        rk= IMP.core.XYZR.get_radius_key()
        #cpf.set_radius_key(rk)
        cpf.set_distance(dist)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, num)
        for i in range(0, len(ps)):
            ps[i].add_attribute(rk, random.uniform(rmin, rmax))
        pc= IMP.container.ListSingletonContainer(ps)
        #pc.show()
        out= IMP.container.ListPairContainer(m)
        #out.show()
        print "searching"
        cps= cpf.get_close_pairs(pc)
        out.set_particle_pairs(IMP.ParticlePairs(cps))
        print "done " + str(out.get_number_of_particle_pairs())
        self._check_close_pairs(ps, dist, rk, out)
        print "Done with all test "+str(out.get_number_of_particle_pairs())
    def do_test_bi_points(self, cpf, num, rmin, rmax):
        #IMP.set_log_level(IMP.SILENT)
        dist= random.uniform(0,2)
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        rk= IMP.core.XYZR.get_radius_key()
        #cpf.set_radius_key(rk)
        cpf.set_distance(dist)
        m=IMP.Model()
        ps= self.create_particles_in_box(m, num)
        for i in range(0, len(ps)):
            ps[i].add_attribute(rk, random.uniform(rmin, rmax))
        pc= IMP.container.ListSingletonContainer(ps)
        psp= self.create_particles_in_box(m, num)
        for i in range(0, len(psp)):
            psp[i].add_attribute(rk, random.uniform(rmin, rmax))
        pcp= IMP.container.ListSingletonContainer(psp)
        pc.show()
        out= IMP.container.ListPairContainer(m)
        out.show()
        cps= cpf.get_close_pairs(pc, pcp)
        out.set_particle_pairs(IMP.ParticlePairs(cps))
        print "done " + str(out.get_number_of_particle_pairs())
        self._check_biclose_pairs(ps, psp,dist, rk, out)
        print "Done with all test "+str(out.get_number_of_particle_pairs())


    def do_test_one(self, cpf):
        #cpf.set_log_level(IMP.SILENT);
        self.do_test_bi_points(cpf, 100, .01,1)
        #cpf.set_log_level(IMP.SILENT);
        self.do_test_points(cpf, 100, 1,1)


    def _check_close_pairs(self, ps, dist, rk, out):
        print "testing results with "
        print " ".join([str((x[0].get_name(), x[1].get_name())) for x in out.get_particle_pairs()])
        found= out.get_particle_pairs()
        for f in found:
            self.assertNotIn(IMP.ParticlePair(f[1], f[0]), found)
            self.assertNotEqual(f[0], f[1])
        for i in range(0, len(ps)):
            for j in range(0,i):
                d= self.get_distance(rk, ps[i], ps[j])
                #d=1000
                if d <= .99*dist:
                    #print "searching for "+str(ps[i].get_name()) + " "\
                    #    + str(ps[j].get_name())
                    #XYZ(ps[
                    self.assertTrue(IMP.ParticlePair(ps[i],ps[j]) in found
                                 or
                                 IMP.ParticlePair(ps[j],ps[i]) in found,
                                 "Pair " +str(ps[i].get_name()) + " " +ps[j].get_name()
                                 + " not found " + str(d) + " " + str(dist))
                    print "found pair " +str(ps[i].get_name()) + " " + str(ps[j].get_name())

    def _check_biclose_pairs(self, ps, ps2, dist, rk, out):
        for i in range(0, len(ps)):
            for j in range(0,len(ps2)):
                if ps[i]== ps2[j]: continue
                #print "testing " + str(ps[i].get_name()) + " " + str(ps2[j].get_name())
                d= self.get_distance(rk, ps[i], ps2[j])
                #print d
                if d <= .95*dist:
                    self.assertTrue(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps2[j])),
                                  "Pair " +str(ps[i]) + " " +str(ps2[j])
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
                    self.assertTrue(out.get_contains_particle_pair(IMP.ParticlePair(ps[i],
                                                                                 ps2[j]))
                                 or out.get_contains_particle_pair(IMP.ParticlePair(ps2[j],
                                                                                 ps[i])),
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
        #IMP.set_log_level(IMP.VERBOSE)
        self.do_test_one(IMP.core.GridClosePairsFinder())

    def test_periodic_grid(self):
        """Testing periodic GridClosePairsFinder"""
        print "grid"
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        cpf= IMP.core.GridClosePairsFinder(bb)
        #cpf.set_log_level(IMP.VERBOSE)
        self.do_test_periodic_points(cpf, bb, 10, 0,1)
    # Don't expect experimental code to work yet
    test_periodic_grid = IMP.test.skip("code is still experimental")(
                                      test_periodic_grid)

    def test_rigid(self):
        "Testing RigidClosePairsFinder"""
        IMP.set_log_level(IMP.SILENT)
        dist= random.uniform(0,2)
        nump=100
        #cpf.set_distance(dist)
        print 'Distance is ' + str(dist)
        #cpf.set_radius_key(rk)
        m=IMP.Model()
        ps= IMP.core.create_xyzr_particles(m, nump, 1)
        rbpsa= IMP.Particles()
        rbpsb= IMP.Particles()
        fps= IMP.Particles()
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
        rba= IMP.core.RigidBody.setup_particle(IMP.Particle(m), IMP.core.XYZs(rbpsa))
        rbb= IMP.core.RigidBody.setup_particle(IMP.Particle(m), IMP.core.XYZs(rbpsb))
        rba.get_particle().set_name("rba")
        rbb.get_particle().set_name("rbb")
        fps=fps+rba.get_members()
        fps=fps+rbb.get_members()
        pc= IMP.container.ListSingletonContainer(fps)
        pc.show()
        out= IMP.container.ListPairContainer(m)
        out.show()
        cpf= IMP.core.RigidClosePairsFinder()
        cpf.set_distance(dist)
        cps=cpf.get_close_pairs(pc)
        out.set_particle_pairs(IMP.ParticlePairs(cps))
        self._check_abiclose_pairs(free_ps, free_ps, dist,
                                   IMP.core.XYZR.get_radius_key(), out)
        self._check_abiclose_pairs(free_ps, rbpsa, dist,
                                   IMP.core.XYZR.get_radius_key(), out)
        self._check_abiclose_pairs(free_ps, rbpsb, dist,
                                   IMP.core.XYZR.get_radius_key(), out)
        self._check_abiclose_pairs(rbpsa, rbpsb, dist,
                                   IMP.core.XYZR.get_radius_key(), out)
        print "Done with all test "+str(out.get_number_of_particle_pairs())
        ps2= self.create_particles_in_box(m, nump)
        rbpsa2= IMP.Particles()
        rbpsb2= IMP.Particles()
        fps2= IMP.Particles()
        for i in range(0, len(ps2)):
            ps2[i].add_attribute(IMP.core.XYZR.get_radius_key(),
                                 random.uniform(0,1))
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

        pc2= IMP.container.ListSingletonContainer(ps2)
        out= IMP.container.ListPairContainer(m)

        cps=cpf.get_close_pairs(pc, pc2)
        out.set_particle_pairs(IMP.ParticlePairs(cps))
        print "done bipartite " + str(out.get_number_of_particle_pairs())
        print ps
        self._check_biclose_pairs(ps, ps2, dist,
                                  IMP.core.XYZR.get_radius_key(), out)

if __name__ == '__main__':
    IMP.test.main()
