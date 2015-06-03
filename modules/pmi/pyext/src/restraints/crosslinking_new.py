"""@namespace IMP.pmi.restraints.crosslinking_new
Restraints for handling crosslinking data.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools
import pdb

class CrossLinkingMassSpectrometryRestraint(object):
    import IMP.isd
    import IMP.pmi.tools
    from math import log

    def __init__(self, representation,
                 CrossLinkDataBase,
                 length,
                 resolution=None,
                 slope=0.0,
                 label="None",
                 filelabel="None",
                 attributes_for_label=None):

        if type(representation) != list:
            representations = [representation]
        else:
            representations = representation

        if not isinstance(CrossLinkDataBase,IMP.pmi.io.crosslink.CrossLinkDataBase):
            raise TypeError("CrossLinkingMassSpectrometryRestraint: CrossLinkDataBase should be an IMP.pmi.io.crosslink.CrossLinkDataBase object")

        self.CrossLinkDataBase=CrossLinkDataBase

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")

        self.m = representations[0].prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'likelihood')
        self.rspsi = IMP.RestraintSet(self.m, 'prior_psi')
        self.rssig = IMP.RestraintSet(self.m, 'prior_sigmas')
        self.rslin = IMP.RestraintSet(self.m, 'linear_dummy_restraints')

        # dummy linear restraint used for Chimera display
        self.linear = IMP.core.Linear(0, 0.0)
        self.linear.set_slope(0.0)
        dps2 = IMP.core.DistancePairScore(self.linear)

        self.label = label
        self.psi_is_sampled = True
        self.sigma_is_sampled = True
        self.psi_dictionary={}
        self.sigma_dictionary={}
        self.xl_list=[]
        self.outputlevel = "low"

        restraints = []

        for xlid in self.CrossLinkDataBase.xlid_iterator():
            new_contribution=True
            for xl in self.CrossLinkDataBase[xlid]:

                r1 = xl[self.CrossLinkDataBase.residue1_key]
                c1 = xl[self.CrossLinkDataBase.protein1_key]
                r2 = xl[self.CrossLinkDataBase.residue2_key]
                c2 = xl[self.CrossLinkDataBase.protein2_key]

                for nstate, r in enumerate(representations):
                    # loop over every state
                    xl[self.CrossLinkDataBase.state_key]=nstate
                    xl[self.CrossLinkDataBase.data_set_name_key]=self.label

                    ps1 = IMP.pmi.tools.select(
                         r,
                         resolution=resolution,
                         name=c1,
                         name_is_ambiguous=False,
                         residue=r1)
                    ps2 = IMP.pmi.tools.select(
                         r,
                         resolution=resolution,
                         name=c2,
                         name_is_ambiguous=False,
                         residue=r2)

                    if len(ps1) > 1:
                        raise ValueError("residue %d of chain %s selects multiple particles %s" % (r1, c1, str(ps1)))
                    elif len(ps1) == 0:
                        print("CrossLinkingMassSpectrometryRestraint: WARNING> residue %d of chain %s is not there" % (r1, c1))
                        midb.write(str(xl) + "\n")
                        continue

                    if len(ps2) > 1:
                        raise ValueError("residue %d of chain %s selects multiple particles %s" % (r2, c2, str(ps2)))
                    elif len(ps2) == 0:
                        print("CrossLinkingMassSpectrometryRestraint: WARNING> residue %d of chain %s is not there" % (r2, c2))
                        midb.write(str(xl) + "\n")
                        continue

                    p1 = ps1[0]
                    p2 = ps2[0]

                    if p1 == p2 and r1 == r2:
                        print("CrossLinkingMassSpectrometryRestraint: WARNING> same particle and same residue, skippin cross-link")
                        continue

                    if new_contribution:
                        print("generating a new crosslink restraint")
                        new_contribution=False
                        dr = IMP.isd.CrossLinkMSRestraint(
                            self.m,
                            length,
                            slope)
                        restraints.append(dr)


                    if self.CrossLinkDataBase.sigma1_key not in xl.keys():
                        sigma1name="SIGMA"
                        xl[self.CrossLinkDataBase.sigma1_key]=sigma1name
                    else:
                        sigma1name=xl[self.CrossLinkDataBase.sigma1_key]
                    sigma1=self.create_sigma(sigma1name)

                    if self.CrossLinkDataBase.sigma2_key not in xl.keys():
                        sigma2name="SIGMA"
                        xl[self.CrossLinkDataBase.sigma2_key]=sigma2name
                    else:
                        sigma2name=xl[self.CrossLinkDataBase.sigma2_key]
                    sigma2=self.create_sigma(sigma2name)

                    if self.CrossLinkDataBase.psi_key not in xl.keys():
                        psiname="PSI"
                        xl[self.CrossLinkDataBase.psi_key]=psiname
                    else:
                        psiname=xl[self.CrossLinkDataBase.psi_key]
                    psi=self.create_psi(psiname)


                    p1i = p1.get_particle_index()
                    xl["Particle1"]=p1
                    p2i = p2.get_particle_index()
                    xl["Particle2"]=p2
                    s1i = sigma1.get_particle().get_index()
                    xl["Particle_sigma1"]=sigma1
                    s2i = sigma2.get_particle().get_index()
                    xl["Particle_sigma2"]=sigma2
                    psii = psi.get_particle().get_index()
                    xl["Particle_psi"]=psi

                    print("B",(p1i, p2i), (s1i, s2i), psii,dr)

                    dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
                    xl["Restraint"]=dr

                    print("--------------")
                    print("CrossLinkingMassSpectrometryRestraint: generating cross-link restraint between")
                    print("CrossLinkingMassSpectrometryRestraint: residue %d of chain %s and residue %d of chain %s" % (r1, c1, r2, c2))
                    print("CrossLinkingMassSpectrometryRestraint: with sigma1 %s sigma2 %s psi %s" % (sigma1name, sigma2name, psiname))
                    print("CrossLinkingMassSpectrometryRestraint: between particles %s and %s" % (p1.get_name(), p2.get_name()))
                    print("==========================================\n")

                    # check if the two residues belong to the same rigid body
                    if(IMP.core.RigidMember.get_is_setup(p1) and
                        IMP.core.RigidMember.get_is_setup(p2) and
                        IMP.core.RigidMember(p1).get_rigid_body() ==
                        IMP.core.RigidMember(p2).get_rigid_body()):
                        xl["IntraRigidBody"]=True
                    else:
                        xl["IntraRigidBody"]=False

                    xl_label=self.CrossLinkDataBase.get_short_cross_link_string(xl)
                    xl["ShortLabel"]=xl_label
                    dr.set_name(xl_label)

                    if p1i != p2i:
                        pr = IMP.core.PairRestraint(self.m, dps2, (p1i, p2i))
                        pr.set_name(xl_label)
                        self.rslin.add_restraint(pr)

                    self.xl_list.append(xl)

                    indb.write(str(xl) + "\n")

        if len(self.xl_list) == 0:
            raise SystemError("CrossLinkingMassSpectrometryRestraint: no crosslink was constructed")

        lw = IMP.isd.LogWrapper(restraints,1.0)
        self.rs.add_restraint(lw)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rspsi)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rssig)
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

    def get_particle_pairs(self):
        ppairs = []
        for i in range(len(self.pairs)):
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            ppairs.append((p0, p1))
        return ppairs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_psi_is_sampled(self, is_sampled=True):
        self.psi_is_sampled = is_sampled

    def set_sigma_is_sampled(self, is_sampled=True):
        self.sigma_is_sampled = is_sampled


    def create_sigma(self, name):
        ''' a nuisance on the structural uncertainty '''
        if name in self.sigma_dictionary:
            return self.sigma_dictionary[name][0]

        sigmainit = 2.0
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
        self.rssig.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                sigma,
                1000000000.0,
                sigmamax,
                sigmamin))
        return sigma

    def create_psi(self, name):
        ''' a nuisance on the inconsistency '''
        if name in self.psi_dictionary:
            return self.psi_dictionary[name][0]

        psiinit=0.25
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

        self.rspsi.add_restraint(
            IMP.isd.UniformPrior(
                self.m,
                psi,
                1000000000.0,
                psimax,
                psimin))

        self.rspsi.add_restraint(IMP.isd.JeffreysRestraint(self.m, psi))

        return psi


    def get_output(self):
        self.m.update()

        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CrossLinkingMassSpectrometryRestraint_Data_Score_" + self.label] = str(score)
        output["CrossLinkingMassSpectrometryRestraint_PriorSig_Score_" +
               self.label] = self.rssig.unprotected_evaluate(None)
        output["CrossLinkingMassSpectrometryRestraint_PriorPsi_Score_" +
                   self.label] = self.rspsi.unprotected_evaluate(None)
        output["CrossLinkingMassSpectrometryRestraint_Linear_Score_" +
               self.label] = self.rslin.unprotected_evaluate(None)
        for xl in self.xl_list:

            xl_label=xl["ShortLabel"]
            ln = xl["Restraint"]
            p0 = xl["Particle1"]
            p1 = xl["Particle2"]
            output["CrossLinkingMassSpectrometryRestraint_Score_" +
                   xl_label] = str(-self.log(ln.unprotected_evaluate(None)))

            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            output["CrossLinkingMassSpectrometryRestraint_Distance_" +
                   xl_label] = str(IMP.core.get_distance(d0, d1))


        for psiname in self.psi_dictionary:
            output["CrossLinkingMassSpectrometryRestraint_Psi_" +
                    str(psiname) + "_" + self.label] = str(self.psi_dictionary[psiname][0].get_scale())

        for sigmaname in self.sigma_dictionary:
            output["CrossLinkingMassSpectrometryRestraint_Sigma_" +
                   str(sigmaname) + "_" + self.label] = str(self.sigma_dictionary[sigmaname][0].get_scale())


        return output

    def get_particles_to_sample(self):
        ps = {}
        if self.sigma_is_sampled:
            for sigmaname in self.sigma_dictionary:
                ps["Nuisances_ISDCrossLinkMS_Sigma_" + str(sigmaname) + "_" + self.label] =\
                    ([self.sigma_dictionary[sigmaname][0]],
                     self.sigma_dictionary[sigmaname][1])

        if self.psi_is_sampled:
            for psiname in self.psi_dictionary:
                if self.psi_dictionary[psiindex][2]:
                    ps["Nuisances_ISDCrossLinkMS_Psi_" +
                        str(psiname) + "_" + self.label] =\
                       ([self.psi_dictionary[psiname][0]], self.psi_dictionary[psiname][1])

        return ps
