import unittest
import IMP
import IMP.test
import IMP.core
import IMP.display

max_score=.02

class CGOptimizerTests(IMP.test.TestCase):
    def check_model(self, m, lsc, lpc):
        s= m.evaluate(False)
        print "score is ", s
        for p0 in lsc.get_particles():
            p0.show()
        self.assert_(s <= max_score)
        for p0 in lsc.get_particles():
            for p1 in lsc.get_particles():
                if p0 != p1:
                    d0= IMP.core.XYZR(p0)
                    d1= IMP.core.XYZR(p1)
                    d= IMP.core.distance(d0,d1)
                    if lpc.get_contains_particle_pair(IMP.ParticlePair(p0,p1)) \
                            or lpc.get_contains_particle_pair(IMP.ParticlePair(p1,p0)) :
                        self.assertInTolerance(d, 0, .2)
                    else:
                        self.assert_(d >= -.2);
    def test_cg_woods_func(self):
        """Check that MCCG can make spheres connect not penetrate"""
        m= IMP.Model()
        n=10
        print 1
        ds= IMP.core.create_xyzr_particles(m, n, 2)
        lsc= IMP.core.ListSingletonContainer(ds.get_particles())
        print 2
        cpf=IMP.core.QuadraticClosePairsFinder()
        print 2.1
        cpc= IMP.core.ClosePairContainer(lsc, .1, cpf)
        print 2.2
        evr= IMP.core.PairsRestraint(IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1)), cpc)
        print 3
        m.add_restraint(evr)
        evr.set_log_level(IMP.WARNING)
        print 4
        lpc= IMP.core.ListPairContainer()
        print "finding pairs"
        for i in range(0,n/2):
            lpc.add_particle_pair(IMP.ParticlePair(lsc.get_particle(2*i),
                                                  lsc.get_particle(2*i+1)))
        d= IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0,1))
        pr= IMP.core.PairsRestraint(d, lpc)
        pr.set_log_level(IMP.WARNING)
        m.add_restraint(pr)
        s= IMP.core.MCCGSampler(m)
        s.set_maximum_score(max_score)
        s.set_number_of_monte_carlo_steps(1000)
        s.set_number_of_conjugate_gradient_steps(100)
        IMP.set_log_level(IMP.TERSE)
        s.set_number_of_attempts(2)
        print "sampling"
        cs=s.sample()
        print "found ", cs.get_number_of_configurations(), cs.get_name()
        for i in range(0, cs.get_number_of_configurations()):
            cs.set_configuration(i)
            nm= "config"+str(i)+".pym"
            w= IMP.display.ChimeraWriter(nm)
            for p in lsc.get_particles():
                d= IMP.core.XYZR(p)
                g= IMP.display.XYZRGeometry(d)
                w.add_geometry(g)
            self.check_model(m, lsc, lpc)




if __name__ == '__main__':
    unittest.main()
