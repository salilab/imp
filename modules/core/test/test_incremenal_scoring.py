import IMP
import IMP.test
import IMP.core
import IMP.container
import random

class Tests(IMP.test.TestCase):
    def test_incr(self):
        """Testing incremental scoring with non-bonded"""
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        ps=[]
        bb= IMP.algebra.get_unit_bounding_box_3d()
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            ps.append(d)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_radius(.1)
            d.set_coordinates_are_optimized(True)
        cpc= IMP.container.ConsecutivePairContainer(ps)
        hps= IMP.core.HarmonicDistancePairScore(1,100)
        #hps.set_log_level(IMP.base.VERBOSE)
        r= IMP.container.PairsRestraint(hps, cpc)
        r.set_name("chain")
        r.set_model(m)
        ls= IMP.container.ListSingletonContainer(ps)
        nbl= IMP.container.ClosePairContainer(ls, 0)
        f=IMP.container.InContainerPairFilter(cpc, True)
        nbl.add_pair_filter(f)
        nbps= IMP.core.SoftSpherePairScore(1)
        rnb= IMP.container.PairsRestraint(nbps, nbl)
        rnb.set_name("NB")
        rnb.set_model(m)
        sf= IMP.core.RestraintsScoringFunction([r, rnb])
        dsf=IMP.core.RestraintsScoringFunction([r.create_decomposition(),
                                                rnb.create_decomposition()])
        isf= IMP.core.IncrementalScoringFunction(ps, [r])
        isf.add_close_pair_score(nbps, 0, ps, [f])
        print "iscore"
        iscore=isf.evaluate(False)
        print "oscore"
        sf.set_log_level(IMP.base.VERBOSE)
        m.set_log_level(IMP.base.VERBOSE)
        oscore=sf.evaluate(False)
        self.assertAlmostEqual(iscore,
                               oscore, delta=.1)
        s= IMP.algebra.get_unit_sphere_3d()
        for i in range(10):
            pi= random.choice(ps)
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
            isf.set_moved_particles([pi.get_particle_index()])
            print "moved", pi
            iscore=isf.evaluate(False)
            dscore=dsf.evaluate(False)
            rscore=sf.evaluate(False)
            print 'scores', iscore, dscore, rscore
            self.assertAlmostEqual(iscore,
                                   dscore, delta=.1)
            self.assertAlmostEqual(iscore,
                                   rscore, delta=.1)
            if i%2 ==0:
                d.set_coordinates(oc)
                isf.reset_moved_particles()
                iscore=isf.evaluate(False)
                dscore=dsf.evaluate(False)
                rscore=sf.evaluate(False)
                print 'scores', iscore, dscore, rscore
                self.assertAlmostEqual(iscore,
                                       dscore, delta=.1)
                self.assertAlmostEqual(iscore,
                                       rscore, delta=.1)
    def test_incr_no_restraints(self):
        """Testing incremental scoring with no restraints"""
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        ps=[]
        bb= IMP.algebra.get_unit_bounding_box_3d()
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            ps.append(d)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_radius(.1)
            d.set_coordinates_are_optimized(True)
        ls= IMP.container.ListSingletonContainer(ps)
        nbl= IMP.container.ClosePairContainer(ls, 0)
        nbps= IMP.core.SoftSpherePairScore(1)
        rnb= IMP.container.PairsRestraint(nbps, nbl)
        rnb.set_name("NB")
        rnb.set_model(m)
        sf= IMP.core.RestraintsScoringFunction([rnb])
        dsf=IMP.core.RestraintsScoringFunction([rnb.create_decomposition()])
        isf= IMP.core.IncrementalScoringFunction(ps, [])
        isf.add_close_pair_score(nbps, 0, ps, [])
        print "iscore"
        iscore=isf.evaluate(False)
        print "oscore"
        sf.set_log_level(IMP.base.VERBOSE)
        m.set_log_level(IMP.base.VERBOSE)
        oscore=sf.evaluate(False)
        self.assertAlmostEqual(iscore,
                               oscore, delta=.1)
        s= IMP.algebra.get_unit_sphere_3d()
        for i in range(10):
            pi= random.choice(ps)
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
            isf.set_moved_particles([pi.get_particle_index()])
            print "moved", pi
            iscore=isf.evaluate(False)
            dscore=dsf.evaluate(False)
            rscore=sf.evaluate(False)
            print 'scores', iscore, dscore, rscore
            self.assertAlmostEqual(iscore,
                                   dscore, delta=.1)
            self.assertAlmostEqual(iscore,
                                   rscore, delta=.1)
            if i%2 ==0:
                d.set_coordinates(oc)
                isf.reset_moved_particles()
                iscore=isf.evaluate(False)
                dscore=dsf.evaluate(False)
                rscore=sf.evaluate(False)
                print 'scores', iscore, dscore, rscore
                self.assertAlmostEqual(iscore,
                                       dscore, delta=.1)
                self.assertAlmostEqual(iscore,
                                       rscore, delta=.1)
    def test_incrnonb(self):
        """Testing incremental scoring"""
        m= IMP.Model()
        #mc.set_log_level(IMP.base.TERSE)
        ps=[]
        bb= IMP.algebra.get_unit_bounding_box_3d()
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            ps.append(d)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_radius(.1)
            d.set_coordinates_are_optimized(True)
        cpc= IMP.container.ConsecutivePairContainer(ps)
        hps= IMP.core.HarmonicDistancePairScore(1,100)
        #hps.set_log_level(IMP.base.VERBOSE)
        r= IMP.container.PairsRestraint(hps, cpc)
        r.set_model(m)
        dsf=IMP.core.RestraintsScoringFunction([r.create_decomposition()])
        sf= IMP.core.RestraintsScoringFunction([r])
        isf= IMP.core.IncrementalScoringFunction(ps, [r])
        isf.set_log_level(IMP.base.VERBOSE)
        print 'initial test'
        iscore=isf.evaluate(False)
        dscore=dsf.evaluate(False)
        rscore=sf.evaluate(False)
        print 'scores', iscore, dscore, rscore
        self.assertAlmostEqual(iscore,
                               dscore, delta=.1)
        self.assertAlmostEqual(iscore,
                               rscore, delta=.1)
        s= IMP.algebra.get_unit_sphere_3d()
        for i in range(10):
            pi= random.choice(ps)
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
            isf.set_moved_particles([pi.get_particle_index()])
            print "moved", pi
            iscore=isf.evaluate(False)
            dscore=dsf.evaluate(False)
            rscore=sf.evaluate(False)
            print 'scores', iscore, dscore, rscore
            self.assertAlmostEqual(iscore,
                                   dscore, delta=.1)
            self.assertAlmostEqual(iscore,
                                   rscore, delta=.1)
            if i%2 ==0:
                d.set_coordinates(oc)
                print 'reseting'
                isf.reset_moved_particles()
                iscore=isf.evaluate(False)
                dscore=dsf.evaluate(False)
                rscore=sf.evaluate(False)
                print 'scores', iscore, dscore, rscore
                self.assertAlmostEqual(iscore,
                                       dscore, delta=.1)
                self.assertAlmostEqual(iscore,
                                       rscore, delta=.1)
        print "resetting"
        for pi in ps:
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
        isf.set_moved_particles(isf.get_movable_particles());
        iscore=isf.evaluate(False)
        dscore=dsf.evaluate(False)
        rscore=sf.evaluate(False)
        print 'scores', iscore, dscore, rscore
        for i in range(10):
            pi= random.choice(ps)
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
            isf.set_moved_particles([pi.get_particle_index()])
            print "moved", pi
            iscore=isf.evaluate(False)
            dscore=dsf.evaluate(False)
            rscore=sf.evaluate(False)
            print 'scores', iscore, dscore, rscore
            self.assertAlmostEqual(iscore,
                                   dscore, delta=.1)
            self.assertAlmostEqual(iscore,
                                   rscore, delta=.1)
            if i%2 ==0:
                d.set_coordinates(oc)
                print 'reseting'
                isf.reset_moved_particles()
                iscore=isf.evaluate(False)
                dscore=dsf.evaluate(False)
                rscore=sf.evaluate(False)
                print 'scores', iscore, dscore, rscore
                self.assertAlmostEqual(iscore,
                                       dscore, delta=.1)
                self.assertAlmostEqual(iscore,
                                       rscore, delta=.1)

    def test_incrigid(self):
        """Testing incremental scoring with rigid bodies"""
        m= IMP.Model()
        #m.set_log_level(IMP.base.SILENT)
        #mc.set_log_level(IMP.base.TERSE)
        ps=[]
        bb= IMP.algebra.get_unit_bounding_box_3d()
        rbs=[]
        for j in range(0,3):
            cps=[]
            for i in range(0,3):
                p= IMP.Particle(m)
                d= IMP.core.XYZR.setup_particle(p)
                ps.append(d)
                d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
                d.set_radius(1)
                d.set_coordinates_are_optimized(True)
                cps.append(p)
            rb= IMP.core.RigidBody.setup_particle(IMP.Particle(m),
                                                  cps)
            rbs.append(rb)
        cpc= IMP.container.ConsecutivePairContainer(rbs)
        hps= IMP.core.HarmonicDistancePairScore(1,100)
        #hps.set_log_level(IMP.base.VERBOSE)
        r= IMP.container.PairsRestraint(hps, cpc)
        r.set_name("C")
        r.set_model(m)
        ls= IMP.container.ListSingletonContainer(ps)
        nbl= IMP.container.ClosePairContainer(ls, 0)
        f=IMP.container.InContainerPairFilter(cpc, True)
        nbl.add_pair_filter(f)
        nbps= IMP.core.SoftSpherePairScore(1)
        rnb= IMP.container.PairsRestraint(nbps, nbl)
        rnb.set_model(m)
        rnb.set_name("NB")
        sf= IMP.core.RestraintsScoringFunction([r, rnb], 1.0, IMP.NO_MAX, "R")
        dsf=IMP.core.RestraintsScoringFunction([r.create_decomposition(),
                                                rnb.create_decomposition()],
                                               1.0, IMP.NO_MAX,
                                               "D")
        isf= IMP.core.IncrementalScoringFunction(rbs, [r], 1.0, IMP.NO_MAX, "I")
        isf.add_close_pair_score(nbps, 0, ps, [f])
        isf.set_log_level(IMP.base.VERBOSE)
        iscore=isf.evaluate(False)
        dscore=dsf.evaluate(False)
        rscore=sf.evaluate(False)
        print 'scores', iscore, dscore, rscore

        dg= IMP.get_dependency_graph(m)
        #IMP.base.show_graphviz(dg)
        self.assertAlmostEqual(iscore,
                               dscore, delta=.1)
        self.assertAlmostEqual(iscore,
                               rscore, delta=.1)
        s= IMP.algebra.get_unit_sphere_3d()
        return
        for i in range(1):
            pi= random.choice(rbs)
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
            isf.set_moved_particles([pi.get_index()])
            print "moved", pi.get_name(), pi.get_particle().get_index()
            iscore=isf.evaluate(False)
            dscore=dsf.evaluate(False)
            rscore=sf.evaluate(False)
            print 'scores', iscore, dscore, rscore
            self.assertAlmostEqual(iscore,
                                   dscore, delta=.1)
            self.assertAlmostEqual(iscore,
                                   rscore, delta=.1)
            if i%2 ==0:
                d.set_coordinates(oc)
                isf.reset_moved_particles()
                iscore=isf.evaluate(False)
                dscore=dsf.evaluate(False)
                rscore=sf.evaluate(False)
                print 'scores', iscore, dscore, rscore
                self.assertAlmostEqual(iscore,
                                       dscore, delta=.1)
                self.assertAlmostEqual(iscore,
                                       rscore, delta=.1)
if __name__ == '__main__':
    IMP.test.main()
