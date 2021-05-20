from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.container
import os

max_score = .02

class Tests(IMP.test.TestCase):

    def check_model(self, m, sf, lsc, lpc):
        s = sf.evaluate(False)
        print("score is ", s)
        for pi0 in lsc.get_contents():
            m.get_particle(pi0).show()
        self.assertLessEqual(s, max_score)
        alli = lpc.get_contents()
        for pi0 in lsc.get_contents():
            for pi1 in lsc.get_contents():
                if pi0 != pi1:
                    d0 = IMP.core.XYZR(m, pi0)
                    d1 = IMP.core.XYZR(m, pi1)
                    d = IMP.core.get_distance(d0, d1)
                    if (pi0, pi1) in alli or (pi1, pi0) in alli:
                        self.assertAlmostEqual(d, 0, delta=.2)
                    else:
                        self.assertGreaterEqual(d, -.2)

    def test_cg_woods_func(self):
        """Check that MCCG can make spheres connect not penetrate"""
        m = IMP.Model()
        n = 10
        print(1)
        ds = IMP.core.create_xyzr_particles(m, n, 2)
        lsc = IMP.container.ListSingletonContainer(m, ds)
        print(2)
        cpf = IMP.core.QuadraticClosePairsFinder()
        print(2.1)
        cpc = IMP.container.ClosePairContainer(lsc, .1, cpf)
        print(2.2)
        evr = IMP.container.PairsRestraint(
            IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0, 1)), cpc)
        print(3)
        evr.set_log_level(IMP.WARNING)
        print(4)
        lpc = IMP.container.ListPairContainer(m)
        print("finding pairs")
        inds = lsc.get_indexes()
        for i in range(0, n // 2):
            print(m.get_particle(inds[2 * i]))
            qp = m.get_particle(inds[2 * i])
            print(qp.get_name())
            print(qp)
            pp = (m.get_particle(inds[2 * i]),
                  m.get_particle(inds[2 * i + 1]))
            print(pp)
            pp2 = (m.get_particle(inds[2 * i]),
                   m.get_particle(inds[2 * i + 1]))
            print(pp2)
            print(str(pp2[0]))
            lpc.add((inds[2*i], inds[2*i+1]))
        d = IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0, 1))
        pr = IMP.container.PairsRestraint(d, lpc)
        pr.set_log_level(IMP.WARNING)
        rs = IMP.RestraintSet(m)
        rs.add_restraint(evr)
        rs.add_restraint(pr)
        sf = IMP.core.RestraintsScoringFunction([rs])

        s = IMP.core.MCCGSampler(m)
        s.set_scoring_function(sf)
        #wos=IMP.WriteParticlesOptimizerState(ds, self.get_tmp_file_name("mccg")+".%1%.imp")
        # wos.set_skip_steps(10)
        # s.add_optimizer_state(wos)
        rs.set_maximum_score(max_score)
        s.set_number_of_monte_carlo_steps(1000)
        s.set_number_of_conjugate_gradient_steps(100)
        IMP.set_log_level(IMP.TERSE)
        s.set_number_of_attempts(2)
        n_trials = 3
        trial = 1
        while(True):
            try:
                print("sampling")
                cs = s.create_sample()
                print("found ", cs.get_number_of_configurations(), cs.get_name())
                for i in range(0, cs.get_number_of_configurations()):
                    cs.load_configuration(i)
                    nm = "config" + str(i) + ".pym"
                    #w= IMP.display.ChimeraWriter(nm)
                    # for p in lsc.get_particles():
                    #    d= IMP.core.XYZR(p)
                    #    g= IMP.core.XYZRGeometry(d)
                    #    w.add_geometry(g)
                    self.check_model(m, sf, lsc, lpc)
                return
            except:
                if trial < n_trials:
                    print("Trial #%d out of %d has failed, trying again" \
                        % (trial, n_trials))
                    trial = trial + 1
                else:
                    raise


if __name__ == '__main__':
    IMP.test.main()
