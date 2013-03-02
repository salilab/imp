import IMP
import IMP.test
import IMP.container
import IMP.display
import math

def nudge_particle(p, sz):
    d = IMP.core.XYZ(p)
    bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-sz,-sz,-sz),
                                   IMP.algebra.Vector3D(sz,sz,sz))
    v= IMP.algebra.get_random_vector_in(bb)
    d.set_coordinates(d.get_coordinates() + v)

class Tests(IMP.test.TestCase):
    """Tests for tunnel scores"""
    def test_connectivity_zero_set(self):
        """Test connectivity zero set"""
        m= IMP.Model()
        ps=[IMP.Particle(m) for i in range(0,15)]
        ds=[IMP.core.XYZR.setup_particle(p) for p in ps]
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        for d in ds:
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_radius(4)
        lsc= IMP.container.ListSingletonContainer(ps)
        cpc= IMP.container.ConnectingPairContainer(lsc, 0)
        hub= IMP.core.HarmonicUpperBound(0,1)
        sdps= IMP.core.SphereDistancePairScore(hub)
        r= IMP.container.PairsRestraint(sdps, cpc)
        m.add_restraint(r)
        w= IMP.display.PymolWriter(self.get_tmp_file_name("connectivity.pym"))
        for d in ds:
            g= IMP.core.XYZRGeometry(d)
            w.add_geometry(g)
        g=IMP.display.RestraintGeometry(r)
        w.add_geometry(g)
        del w
        m.evaluate(False)
        for pr in cpc.get_particle_pairs():
            dist= IMP.core.get_distance(IMP.core.XYZR(pr[0]),
                                        IMP.core.XYZR(pr[1]))
        self.assertEqual(m.evaluate(False), 0)
    def test_score(self):
        """Test connectivity"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        ps= IMP.core.create_xyzr_particles(m, 10, .1)
        for p in ps:
            p.set_coordinates_are_optimized(True)
        lsc=IMP.container.ListSingletonContainer(ps)
        cpc=IMP.container.ConnectingPairContainer(lsc, .1)
        #m.add_restraint(pr)
        m.evaluate(False)
        for pp in cpc.get_particle_pairs():
            print pp
            # for p0 in ps:
            #     for p1 in ps:
            #         if p0 != p1:
            #             d0= IMP.core.XYZR(p0)
            #             d1= IMP.core.XYZR(p1)
            #             if IMP.core.get_distance(d0, d1) < 0:
            #                 print str(p0)+" " +str(p1)
            #                 self.assert_(cpc.get_contains_particle_pair(ParticlePair(p0, p1))
            #                 or cpc.get_contains_particle_pair(ParticlePair(p1, p0)))
        cg= IMP.core.ConjugateGradients(m)
        ub= IMP.core.HarmonicUpperBound(0, 1)
        sd= IMP.core.DistancePairScore(ub)
        pr= IMP.container.PairsRestraint(sd, cpc)
        m.add_restraint(pr)
        print "added"
        print pr.evaluate(False)
        cg.set_threshold(.0001)
        for i in range(10):
            try:
                cg.optimize(100)
            except IMP.ValueException: # Catch CG failure
                pass
            if pr.evaluate(False) <= .0001:
                break
            # Nudge the particles a little to escape a local minimum
            for p in ps:
                nudge_particle(p, 1.0)
        for pp in cpc.get_particle_pairs():
            print pp
            print
        for p0 in ps:
            for p1 in ps:
                if p0 != p1:
                    d0= IMP.core.XYZR(p0.get_particle())
                    d1= IMP.core.XYZR(p1.get_particle())
                    print d0
                    print d1
                    print (p0.get_particle(), p1.get_particle())
                    self.assertLessEqual(IMP.core.get_distance(d0, d1), 0.1)
                    #self.assert_(cpc.get_contains_particle_pair((p0.get_particle(), p1.get_particle()))
                                 #or cpc.get_contains_particle_pair((p1.get_particle(), p0.get_particle())))



if __name__ == '__main__':
    IMP.test.main()
