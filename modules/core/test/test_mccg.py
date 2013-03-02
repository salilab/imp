import IMP
import IMP.test
import IMP.core
import IMP.container
import os

max_score=.02

class Tests(IMP.test.TestCase):
    def check_model(self, m, lsc, lpc):
        s= m.evaluate(False)
        print "score is ", s
        for p0 in lsc.get_particles():
            p0.show()
        self.assertLessEqual(s, max_score)
        alli= lpc.get_particle_pairs()
        for p0 in lsc.get_particles():
            for p1 in lsc.get_particles():
                if p0 != p1:
                    d0= IMP.core.XYZR(p0)
                    d1= IMP.core.XYZR(p1)
                    d= IMP.core.get_distance(d0,d1)
                    if (p0,p1) in alli or (p1,p0) in alli:
                        self.assertAlmostEqual(d, 0, delta=.2)
                    else:
                        self.assertGreaterEqual(d, -.2)
    def test_cg_woods_func(self):
        """Check that MCCG can make spheres connect not penetrate"""
        m= IMP.Model()
        n=10
        print 1
        ds= IMP.core.create_xyzr_particles(m, n, 2)
        lsc= IMP.container.ListSingletonContainer(ds)
        print 2
        cpf=IMP.core.QuadraticClosePairsFinder()
        print 2.1
        cpc= IMP.container.ClosePairContainer(lsc, .1, cpf)
        print 2.2
        evr= IMP.container.PairsRestraint(IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1)), cpc)
        print 3
        m.add_restraint(evr)
        evr.set_log_level(IMP.WARNING)
        print 4
        lpc= IMP.container.ListPairContainer(m)
        print "finding pairs"
        for i in range(0,n/2):
            print lsc.get_particle(2*i)
            qp=lsc.get_particle(2*i)
            print qp.get_name()
            print qp
            pp=(lsc.get_particle(2*i),
                                lsc.get_particle(2*i+1))
            print pp
            pp2=(lsc.get_particle(2*i),
                 lsc.get_particle(2*i+1))
            print pp2
            print str(pp2[0])
            lpc.add_particle_pair(pp2)
        d= IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0,1))
        pr= IMP.container.PairsRestraint(d, lpc)
        pr.set_log_level(IMP.WARNING)
        m.add_restraint(pr)
        s= IMP.core.MCCGSampler(m)
        #wos=IMP.WriteParticlesOptimizerState(ds, self.get_tmp_file_name("mccg")+".%1%.imp")
        #wos.set_skip_steps(10)
        #s.add_optimizer_state(wos)
        m.set_maximum_score(max_score)
        s.set_number_of_monte_carlo_steps(1000)
        s.set_number_of_conjugate_gradient_steps(100)
        IMP.base.set_log_level(IMP.base.TERSE)
        s.set_number_of_attempts(2)
        print "sampling"
        cs=s.get_sample()
        print "found ", cs.get_number_of_configurations(), cs.get_name()
        for i in range(0, cs.get_number_of_configurations()):
            cs.load_configuration(i)
            nm= "config"+str(i)+".pym"
            #w= IMP.display.ChimeraWriter(nm)
            #for p in lsc.get_particles():
            #    d= IMP.core.XYZR(p)
            #    g= IMP.core.XYZRGeometry(d)
            #    w.add_geometry(g)
            self.check_model(m, lsc, lpc)



if __name__ == '__main__':
    IMP.test.main()
