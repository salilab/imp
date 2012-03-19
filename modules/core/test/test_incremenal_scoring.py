import IMP
import IMP.test
import IMP.core
import IMP.container
import random

class Tests(IMP.test.TestCase):
    def _test_incr(self):
        """Testing incremental scoring with non-bonded"""
        m= IMP.Model()
        m.set_log_level(IMP.SILENT)
        #mc.set_log_level(IMP.TERSE)
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
        #hps.set_log_level(IMP.VERBOSE)
        r= IMP.container.PairsRestraint(hps, cpc)
        r.set_model(m)
        ls= IMP.container.ListSingletonContainer(ps)
        nbl= IMP.container.ClosePairContainer(ls, 0)
        f=IMP.container.InContainerPairFilter(cpc)
        nbl.add_pair_filter(f)
        nbps= IMP.core.SoftSpherePairScore(1)
        rnb= IMP.container.PairsRestraint(nbps, nbl)
        rnb.set_model(m)
        sf= IMP.RestraintsScoringFunction([r, rnb])
        isf= IMP.core.IncrementalScoringFunction([r])
        isf.add_close_pair_score(nbps, 0, ps, [f])
        self.assertAlmostEqual(isf.evaluate(False),
                               sf.evaluate(False), .1)
        s= IMP.algebra.get_unit_sphere_3d()
        for i in range(10):
            d.set_coordinates(oc)
            pi= random.select(ps)
            d= IMP.core.XYZ(pi)
            oc= d.get_coordinates()
            nc= oc+ IMP.algebra.get_random_vector_in(s)
            d.set_coordinates(nc)
            isf.set_moved_particle([pi])
            self.assertAlmostEqual(isf.evaluate(False),
                                   sf.evaluate(False), .1)
            if i%2 ==0:
                isf.reset_moved_particle()
                self.assertAlmostEqual(isf.evaluate(False),
                                       sf.evaluate(False), .1)
    def test_incrnonb(self):
        """Testing incremental scoring"""
        m= IMP.Model()
        #mc.set_log_level(IMP.TERSE)
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
        #hps.set_log_level(IMP.VERBOSE)
        r= IMP.container.PairsRestraint(hps, cpc)
        r.set_model(m)
        dsf=IMP.RestraintsScoringFunction([r.create_decomposition()])
        sf= IMP.RestraintsScoringFunction([r])
        isf= IMP.core.IncrementalScoringFunction([r])
        isf.set_log_level(IMP.VERBOSE)
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
            isf.set_moved_particles([pi])
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

if __name__ == '__main__':
    IMP.test.main()
