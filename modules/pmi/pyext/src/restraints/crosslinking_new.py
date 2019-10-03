"""@namespace IMP.pmi.restraints.crosslinking_new
Restraints for handling crosslinking data. This temporary module will be soon
deprecated and merged with IMP.pmi.restraints.crosslinking
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools
import IMP.pmi.restraints.crosslinking
import pdb

class DisulfideCrossLinkRestraint(object):
    def __init__(self, representation_or_hier,
                 selection_tuple1,
                 selection_tuple2,
                 length=6.5,
                 resolution=1,
                 slope=0.01,
                 label="None"):

        self.m = representation_or_hier.get_model()
        ps1 = IMP.pmi.tools.select_by_tuple_2(representation_or_hier,
                                              selection_tuple1,
                                              resolution=resolution)
        ps2 = IMP.pmi.tools.select_by_tuple_2(representation_or_hier,
                                              selection_tuple2,
                                              resolution=resolution)

        self.rs = IMP.RestraintSet(self.m, 'likelihood')
        self.rslin = IMP.RestraintSet(self.m, 'linear_dummy_restraints')

        # dummy linear restraint used for Chimera display
        self.linear = IMP.core.Linear(0, 0.0)
        self.linear.set_slope(0.0)
        dps2 = IMP.core.DistancePairScore(self.linear)

        self.label = label
        self.psi_dictionary={}
        self.sigma_dictionary={}
        self.psi_is_sampled = False
        self.sigma_is_sampled = False
        self.xl={}

        if len(ps1) > 1 or len(ps1) == 0:
            raise ValueError("DisulfideBondRestraint: ERROR> first selection pattern selects multiple particles or sero particles")

        if len(ps2) > 1 or len(ps2) == 0:
            raise ValueError("DisulfideBondRestraint: ERROR> second selection pattern selects multiple particles or sero particles")

        p1 = ps1[0]
        p2 = ps2[0]

        sigma=self.create_sigma("SIGMA_DISULFIDE_BOND")
        psi=self.create_psi("PSI_DISULFIDE_BOND")

        p1i = p1.get_index()
        p2i = p2.get_index()
        si = sigma.get_particle().get_index()
        psii = psi.get_particle().get_index()

        dr = IMP.isd.CrossLinkMSRestraint(
                                    self.m,
                                    length,
                                    slope)

        dr.add_contribution((p1i, p2i), (si, si), psii)

        if p1i != p2i:
            pr = IMP.core.PairRestraint(self.m, dps2, (p1i, p2i))
            pr.set_name("DISULFIDE_BOND_"+self.label)
            self.rslin.add_restraint(pr)

        lw = IMP.isd.LogWrapper([dr],1.0)
        self.rs.add_restraint(lw)

        self.xl["Particle1"]=p1
        self.xl["Particle2"]=p2
        self.xl["Sigma"]=sigma
        self.xl["Psi"]=psi

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rslin)

    def get_hierarchies(self):
        return self.prot

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def get_restraint_for_rmf(self):
        return self.rslin

    def get_restraints(self):
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

    def set_psi_is_sampled(self, is_sampled=True):
        self.psi_is_sampled = is_sampled

    def set_sigma_is_sampled(self, is_sampled=True):
        self.sigma_is_sampled = is_sampled


    def create_sigma(self, name):
        ''' a nuisance on the structural uncertainty '''
        if name in self.sigma_dictionary:
            return self.sigma_dictionary[name][0]

        sigmainit = 1.0
        sigmaminnuis = 0.0000001
        sigmamaxnuis = 1000.0
        sigmamin = 0.01
        sigmamax = 100.0
        sigmatrans = 0.5
        sigma = IMP.pmi.tools.SetupNuisance(self.m, sigmainit,
                                                 sigmaminnuis, sigmamaxnuis, self.sigma_is_sampled).get_particle()
        self.sigma_dictionary[name] = (
            sigma,
            sigmatrans,
            self.sigma_is_sampled)

        return sigma

    def create_psi(self, name):
        ''' a nuisance on the inconsistency '''
        if name in self.psi_dictionary:
            return self.psi_dictionary[name][0]

        psiinit=0.001
        psiminnuis = 0.0000001
        psimaxnuis = 0.4999999
        psimin = 0.01
        psimax = 0.49
        psitrans = 0.1
        psi = IMP.pmi.tools.SetupNuisance(self.m, psiinit,
                                               psiminnuis, psimaxnuis,
                                               self.psi_is_sampled).get_particle()
        self.psi_dictionary[name] = (
            psi,
            psitrans,
            self.psi_is_sampled)

        return psi


    def get_output(self):
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["DisulfideBondRestraint_Data_Score_" + self.label] = str(score)
        output["DisulfideBondRestraint_Linear_Score_" +
               self.label] = self.rslin.unprotected_evaluate(None)
        return output

    def get_particles_to_sample(self):
        raise NotImplementedError(" ")
