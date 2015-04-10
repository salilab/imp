"""@namespace IMP.pmi.restraints.saxs
Restraints for handling small angle x-ray (SAXS) data.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container


class SAXSISDRestraint(object):

    import IMP.saxs
    import IMP.isd
    import IMP.isd2
    import IMP.pmi.tools

    def __init__(self, representation, profile, resolution=0, weight=1,
                 ff_type=IMP.saxs.HEAVY_ATOMS):

        self.m = representation.prot.get_model()
        self.label = "None"
        self.rs = IMP.RestraintSet(self.m, 'saxs')

        self.taumaxtrans = 0.05
        self.prof = IMP.saxs.Profile(profile)

        self.atoms = IMP.pmi.tools.select(
            representation,
            resolution=resolution)

        # gamma nuisance
        self.gamma = IMP.pmi.tools.SetupNuisance(
            self.m, 1., 0., None, False).get_particle()

        # sigma nuisance
        self.sigma = IMP.pmi.tools.SetupNuisance(self.m, 10.0, 0., None, False
                                                 ).get_particle()

        # tau nuisance, optimized
        self.tau = IMP.pmi.tools.SetupNuisance(self.m, 1., 0., None, False,
                                               ).get_particle()

        #c1 and c2, optimized
        self.c1 = IMP.pmi.tools.SetupNuisance(self.m, 1.0, 0.95, 1.05,
                                              True).get_particle()
        self.c2 = IMP.pmi.tools.SetupNuisance(self.m, 0.0, -2., 4.,
                                              True).get_particle()

        #weight, optimized
        self.w = IMP.pmi.tools.SetupWeight(self.m).get_particle()
        IMP.isd.Weight(self.w).set_weights_are_optimized(True)

        # take identity covariance matrix for the start
        self.cov = [[1 if i == j else 0 for j in range(self.prof.size())]
                    for i in range(self.prof.size())]

        print("create saxs restraint")
        self.saxs = IMP.isd2.SAXSRestraint(self.prof, self.sigma, self.tau,
                                           self.gamma, self.w, self.c1, self.c2)
        self.saxs.add_scatterer(self.atoms, self.cov, ff_type)

        self.rs.add_restraint(self.saxs)
        self.rs.set_weight(weight)

        # self.saxs_stuff={'nuis':(sigma,gamma),'cov':cov,
        #        'exp':prof,'th':tmp}

        self.rs2 = IMP.RestraintSet(self.m, 'jeffreys')
        # jeffreys restraints for nuisances
        j1 = IMP.isd.JeffreysRestraint(self.m, self.sigma)
        self.rs2.add_restraint(j1)
        j2 = IMP.isd.JeffreysRestraint(self.m, self.tau)
        self.rs2.add_restraint(j2)
        j3 = IMP.isd.JeffreysRestraint(self.m, self.gamma)
        self.rs2.add_restraint(j3)

    def optimize_sigma(self):
        """set sigma to the value that maximizes its conditional likelihood"""
        from math import sqrt
        self.m.update()
        sigma2hat = self.saxs.get_sigmasq_scale_parameter() \
            / (self.saxs.get_sigmasq_shape_parameter() + 1)
        IMP.isd.Scale(self.sigma).set_scale(sqrt(sigma2hat))

    def optimize_gamma(self):
        """set gamma to the value that maximizes its conditional likelihood"""
        from math import exp
        self.m.update()
        gammahat = exp(self.saxs.get_loggamma_variance_parameter()
                       * self.saxs.get_loggamma_jOg_parameter())
        IMP.isd.Scale(self.gamma).set_scale(gammahat)

    def logspace(self, a, b, num=100):
        """mimick numpy's logspace function"""
        for i in range(num):
            val = a + float(b - a) / float(num - 1) * i
            yield 10 ** val

    def optimize_tau(self, ltaumin=-2, ltaumax=3, npoints=100):
        from math import log
        import IMP.atom
        values = []
        self.m.update()
        IMP.atom.write_pdb(self.atoms, 'tauvals.pdb')
        fl = open('tauvals.txt', 'w')
        for tauval in self.logspace(ltaumin, ltaumax, npoints):
            IMP.isd.Scale(self.tau).set_scale(tauval)
            try:
                values.append((self.m.evaluate(False), tauval))
            except:
                pass
            fl.write('%G %G\n' % (values[-1][1], values[-1][0]))
        values.sort()
        ltcenter = log(values[0][1]) / log(10)
        spacing = (ltaumax - ltaumin) / float(npoints)
        values = []
        for tauval in self.logspace(
            ltcenter - 2 * spacing, ltcenter + 2 * spacing,
                npoints):
            IMP.isd.Scale(self.tau).set_scale(tauval)
            values.append((self.m.evaluate(False), tauval))
            fl.write('%G %G\n' % (values[-1][1], values[-1][0]))
        values.sort()
        IMP.isd.Scale(self.tau).set_scale(values[0][1])

    def draw_sigma(self):
        """draw 1/sigma2 from gamma distribution"""
        self.m.update()
        self.saxs.draw_sigma()

    def draw_gamma(self):
        """draw gamma from lognormal distribution"""
        self.m.update()
        self.saxs.draw_gamma()

    def update_covariance_matrix(self):
        c1 = IMP.isd.Nuisance(self.c1).get_nuisance()
        c2 = IMP.isd.Nuisance(self.c2).get_nuisance()
        #tau = IMP.isd.Nuisance(self.tau).get_nuisance()
        tau = 1.0
        self.cov = IMP.isd2.compute_relative_covariance(self.atoms, c1, c2,
                                                        tau, self.prof)
        # for i in xrange(len(self.cov)):
        #    for j in xrange(len(self.cov)):
        #        self.cov[i][j] = self.cov[i][j]/tau**2
        self.saxs.set_cov(0, self.cov)

    def write_covariance_matrix(self, fname):
        fl = open(fname, 'w')
        for line in self.cov:
            for i in line:
                fl.write('%G ' % i)
            fl.write('\n')

    def get_gamma_value(self):
        return self.gamma.get_scale()

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.m.add_restraint(self.rs)
        self.m.add_restraint(self.rs2)

    def set_taumaxtrans(self, taumaxtrans):
        self.taumaxtrans = taumaxtrans

    def get_particles_to_sample(self):
        ps = {}
        # ps["Nuisances_SAXSISDRestraint_Tau_" +
        #    self.label] = ([self.tau], self.taumaxtrans)
        return ps

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        score2 = self.rs2.unprotected_evaluate(None)
        output["_TotalScore"] = str(score + score2)

        output["SAXSISDRestraint_Likelihood_" + self.label] = str(score)
        output["SAXSISDRestraint_Prior_" + self.label] = str(score2)
        output["SAXSISDRestraint_Sigma_" +
               self.label] = str(self.sigma.get_scale())
        output["SAXSISDRestraint_Tau_" +
               self.label] = str(self.tau.get_scale())
        output["SAXSISDRestraint_Gamma_" +
               self.label] = str(self.gamma.get_scale())
        return output
