import unittest
import IMP, IMP.test
import IMP.core
import random


class TestNBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)
        self.rk= IMP.FloatKey("radius")

    def test_quadratic(self):
        """Test quadratic NBL"""
        ss='IMP.core.AllNonbondedListScoreState(md, self.rk, IMP.core.AllNonbondedListScoreState.QUADRATIC)'
        #eval(ss)
        self.do_test_all(ss)


    def test_grid(self):
        """Test grid NBL"""
        #IMP.set_log_level(IMP.VERBOSE)
        ss='IMP.core.AllNonbondedListScoreState(md, self.rk, IMP.core.AllNonbondedListScoreState.GRID)'
        self.do_test_all(ss)

    def test_bbox(self):
        """Test bbox NBL"""
        try:
            q= IMP.core.AllNonbondedListScoreState.BBOX
        except:
            print "No CGAL support"
        else:
            ss= 'IMP.core.AllNonbondedListScoreState(md, self.rk, IMP.core.AllNonbondedListScoreState.BBOX)'
            self.do_test_all(ss)

    def test_default(self):
        """Test default NBL"""
        ss= 'IMP.core.AllNonbondedListScoreState(md, self.rk)'
        self.do_test_all(ss)

    def test_bipartite_quadratic(self):
        """Test bipartite quadratic NBL"""
        ss='IMP.core.BipartiteNonbondedListScoreState(md, self.rk, IMP.core.BipartiteNonbondedListScoreState.QUADRATIC)'
        self.do_test_all_bi(ss)

    def test_bipartite_bbox(self):
        """Test bipartite bbox NBL"""
        try:
            q= IMP.core.BipartiteNonbondedListScoreState.BBOX
        except:
            print "No CGAL support"
        else:
            ss= 'IMP.core.BipartiteNonbondedListScoreState(md, self.rk, IMP.core.BipartiteNonbondedListScoreState.BBOX)'
            self.do_test_all_bi(ss)



    def do_test_all(self, ss):
        self.do_test_bl(ss)
        self.do_test_distfilt(ss)
        self.do_test_spheres(ss)

    def do_test_all_bi(self, ss):
        self.do_test_bi(ss)
        self.do_test_bi_update(ss)

    def make_spheres(self, m, num, lbv, ubv, minr, maxr):
        ps=self.create_particles_in_box(m, num, lbv, ubv)
        for p in ps:
            p.add_attribute(self.rk, random.uniform(minr, maxr), False)
        return ps

    def do_test_update(self, ss):
        m= IMP.Model()
        self.make_spheres(m, 20, [0,0,0], [10,10,10], .1, 1)
        ds=[]
        for p in m.get_particles():
            ds.append(IMP.core.XYZDecorator.cast(p))
        md=5
        s=eval(ss)
        s.set_particles(m.get_particles())
        m.add_score_state(s)
        o= IMP.test.ConstPairScore(1)
        r= IMP.core.NonbondedRestraint(o, s)
        m.add_restraint(r)
        # use the internal checks
        for i in range(0,10):
            score= m.evaluate(False)
            for d in ds:
                d.set_coordinates(IMP.random_vector_in_sphere(d.get_coordinates(),
                                                              2.0))

    def do_test_bl(self, ss):
        """Test the bond decorator list"""
        m= IMP.Model()
        bds=[]
        pts=self.make_spheres(m, 20, [0,10,10], [10,20,20], .1, 1)
        for p in pts:
            bds.append(IMP.core.BondedDecorator.create(p))
        for i in range(1,len(pts)):
            IMP.core.custom_bond(bds[i-1], bds[i], 1, .1)
        md= 20
        s=eval(ss)
        s.set_particles(pts)
        b= IMP.core.BondDecoratorListScoreState(pts)
        s.add_bonded_list(b)
        m.add_score_state(s)
        o= IMP.test.ConstPairScore(1)
        r= IMP.core.NonbondedRestraint(o, s)
        os=IMP.test.ConstUnaryFunction(100)
        print os.evaluate(6)
        br= IMP.core.BondDecoratorRestraint(os, b)
        m.add_restraint(r)
        m.add_restraint(br)
        score= m.evaluate( False )
        print "Score with bonds is " + str(score)
        self.assertEqual(score, 190+1900-19, "Wrong score")

    def do_test_distfilt(self, ss):
        """Test filtering based on distance in nonbonded list"""
        #IMP.set_log_level(IMP.TERSE)
        m= IMP.Model()
        ps=IMP.Particles()
        ps= self.create_particles_in_box(m, 20, [0,0,0], [10,10,10])
        pts= self.create_particles_in_box(m, 20, [160,160,160], [170,170,170])
        for p in pts:
            ps.append(p)
        md=15
        s=eval(ss)
        s.set_particles(ps)
        m.add_score_state(s)
        o= IMP.test.ConstPairScore(1)
        r= IMP.core.NonbondedRestraint(o, s)
        m.add_restraint(r)
        score= m.evaluate(False)
        self.assertEqual(score, 2*190, "Wrong score")

        m.remove_particle(ps[3].get_index())
        self.assert_(not ps[3].get_is_active(), "Particle not inactive")
        ps=None
        score= m.evaluate(False)
        print score
        self.assertEqual(score, 171+190, "Wrong score with removal")

        #for p in s.get_particles():
        #   self.assert_(p.get_is_active(), "Inactive particle not removed")

        p= IMP.Particle()
        m.add_particle(p)
        print "Index is " +str(p.get_index().get_index())
        d=IMP.core.XYZDecorator.create(p)
        d.set_coordinates_are_optimized(True)
        d.set_coordinates(IMP.random_vector_in_box(IMP.Vector3D(0,0,0),
                                                   IMP.Vector3D(10,10,10)))
        nps= IMP.Particles([p])
        s.add_particles(nps)
        score= m.evaluate(False)
        print score
        self.assertEqual(score, 2*190, "Wrong score after insert")


    def do_test_bi(self, ss):
        """Test the bipartite nonbonded list and restraint which uses it"""
        m= IMP.Model()
        ps0=self.make_spheres(m, 5, [0,0,0], [10,10,10], .1, 1)
        ps1=self.make_spheres(m, 5, [0,0,0], [10,10,10], .1, 1)
        md=15
        s=eval(ss)
        #IMP.QuadraticBipartiteNonbondedListScoreState(IMP.FloatKey(),
        #                                              ps0, ps1)
        s.set_particles(ps0, ps1)
        m.add_score_state(s)
        o= IMP.test.ConstPairScore(1)
        r= IMP.core.NonbondedRestraint(o, s)
        m.add_restraint(r)
        score= m.evaluate(False)
        print score
        self.assertEqual(score, 25, "Wrong score")

    def do_test_bi_update(self, ss):
        """Test the bipartite nonbonded list and restraint which uses it"""
        m= IMP.Model()
        ps0=self.make_spheres(m, 5, [0,0,0], [10,10,10], .1, 1)
        ps1=self.make_spheres(m, 5, [0,0,0], [10,10,10], .1, 1)
        md=5
        s=eval(ss)
        #IMP.QuadraticBipartiteNonbondedListScoreState(IMP.FloatKey(),
        #                                              ps0, ps1)
        s.set_particles(ps0, ps1)
        m.add_score_state(s)
        o= IMP.test.ConstPairScore(1)
        r= IMP.core.NonbondedRestraint(o, s)
        m.add_restraint(r)
        for i in range(0,20):
            score= m.evaluate(False)
            for p in ps0:
                d= IMP.core.XYZDecorator.cast(p)
                d.set_coordinates(IMP.random_vector_in_sphere(d.get_coordinates(),
                                                              1))
            for p in ps1:
                d= IMP.core.XYZDecorator.cast(p)
                d.set_coordinates(IMP.random_vector_in_sphere(d.get_coordinates(),
                                                              1))


    def do_test_spheres(self, ss):
        """Test the nonbonded list of spheres (collision detection)"""
        m= IMP.Model()
        ps= self.make_spheres(m, 20, IMP.Vector3D(0,0,0),
                                IMP.Vector3D(10,10,10), 0, 100)
        md= 1
        s=eval(ss)
        s.set_particles(m.get_particles())
        #IMP.core.AllNonbondedListScoreState(rk, m.get_particles())
        m.add_score_state(s)
        sd= IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1),
                                             self.rk)
        r= IMP.core.NonbondedRestraint(sd, s)
        m.add_restraint(r)
        score= m.evaluate(False)
        opt= IMP.core.ConjugateGradients()
        opt.set_model(m)
        #IMP.set_log_level(IMP.TERSE)
        score =opt.optimize(10000)
        print score
        #for p in m.get_particles():
        #    dp= IMP.core.XYZDecorator.cast(p)
        #    print ".sphere "+str(dp.get_x()) + " " + str(dp.get_y())\
        #        + " " + str(dp.get_z()) + " " +str( p.get_value(self.rk))
        for p in m.get_particles():
            #p.show()
            dp= IMP.core.XYZDecorator.cast(p)
            for q in m.get_particles():
                dq= IMP.core.XYZDecorator.cast(q)
                if p.get_index() != q.get_index():
                    d = IMP.core.distance(dp,dq)
                    rd= p.get_value(self.rk) + q.get_value(self.rk)
                    if rd > d:
                        p.show()
                        q.show()
                        print d
                        print rd
                    # Allow a little extra, for imperfect optimization:
                    self.assert_(rd <= d + 1e-3,
                                 "Some spheres are not repelled")


if __name__ == '__main__':
    unittest.main()
