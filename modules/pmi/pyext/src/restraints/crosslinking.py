"""@namespace IMP.pmi.restraints.crosslinking
Restraints for handling crosslinking data.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.isd
import IMP.container
import IMP.pmi.tools
import IMP.pmi.output
from math import log
from collections import defaultdict
import itertools

class CrossLinkingMassSpectrometryRestraint(object):
    """Setup cross-link distance restraints from mass spectrometry data.
    The noise in the data and the structural uncertainty of cross-linked amino-acids
    is inferred using Bayes theory of probabilty
    \note Wraps an IMP::isd::CrossLinkMSRestraint
    """
    def __init__(self, representation=None,
                 root_hier=None,
                 CrossLinkDataBase=None,
                 length=10.0,
                 resolution=None,
                 slope=0.02,
                 label="None",
                 filelabel="None",
                 attributes_for_label=None):
        """Constructor.
        @param representation DEPRECATED The IMP.pmi.representation.Representation
                object that contain the molecular system
        @param root_hier The canonical hierarchy containing all the states
        @param CrossLinkDataBase The IMP.pmi.io.crosslink.CrossLinkDataBase
                object that contains the cross-link dataset
        @param length maximal cross-linker lenght (including the residue sidechains)
        @param resolution what representation resolution shoult the cross-link
                restraint be applied to.
        @param slope  The slope of a distance-linear scoring function that
               funnel the score when the particles are
               too far away. Suggested value 0.02.
        @param label the extra text to label the restraint so that it is
                searchable in the output
        @param filelabel automaticlly generated file containing missing/included/excluded
                cross-links will be labeled using this text
        @param attributes_for_label
        """

        use_pmi2 = True
        if representation is not None:
            use_pmi2 = False
            if type(representation) != list:
                representations = [representation]
            else:
                representations = representation
            self.m = representations[0].prot.get_model()
        elif root_hier is not None:
            self.m = root_hier.get_model()
        else:
            raise Exception("You must pass either representation or root_hier")

        if CrossLinkDataBase is None:
            raise Exception("You must pass a CrossLinkDataBase")
        if not isinstance(CrossLinkDataBase,IMP.pmi.io.crosslink.CrossLinkDataBase):
            raise TypeError("CrossLinkingMassSpectrometryRestraint: CrossLinkDataBase should be an IMP.pmi.io.crosslink.CrossLinkDataBase object")
        self.CrossLinkDataBase = CrossLinkDataBase

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")


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

                if use_pmi2:
                    iterlist = range(len(IMP.atom.get_by_type(root_hier,IMP.atom.STATE_TYPE)))
                else:
                    iterlist = representations
                for nstate, r in enumerate(iterlist):
                    # loop over every state
                    xl[self.CrossLinkDataBase.state_key]=nstate
                    xl[self.CrossLinkDataBase.data_set_name_key]=self.label

                    if not use_pmi2:
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
                    else:
                        ps1 = IMP.atom.Selection(root_hier,
                                                 state_index=nstate,
                                                 molecule=c1,
                                                 residue_index=r1,
                                                 resolution=resolution).get_selected_particles()
                        ps1 = [IMP.atom.Hierarchy(p) for p in ps1] #to make it work below
                        ps2 = IMP.atom.Selection(root_hier,
                                                 state_index=nstate,
                                                 molecule=c2,
                                                 residue_index=r2,
                                                 resolution=resolution).get_selected_particles()
                        ps2 = [IMP.atom.Hierarchy(p) for p in ps2]

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
                        print("CrossLinkingMassSpectrometryRestraint: WARNING> same particle and same residue, skipping cross-link")
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
        """ Add the restraint to the model so that it is evaluated """
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rspsi)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rssig)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rslin)

    def get_hierarchies(self):
        """ get the hierarchy """
        return self.prot

    def get_restraint_sets(self):
        """ get the restraint set """
        return self.rs

    def get_restraint(self):
        """ get the restraint set  (redundant with get_restraint_sets)"""
        return self.rs

    def get_restraint_for_rmf(self):
        """ get the dummy restraints to be displayed in the rmf file """
        return self.rslin

    def get_restraints(self):
        """ get the restraints in a list """
        rlist = []
        for r in self.rs.get_restraints():
            rlist.append(IMP.core.PairRestraint.get_from(r))
        return rlist

    def get_particle_pairs(self):
        """ Get a list of tuples containing the particle pairs """
        ppairs = []
        for i in range(len(self.pairs)):
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            ppairs.append((p0, p1))
        return ppairs

    def set_output_level(self, level="low"):
        """ Set the output level of the output """
        self.outputlevel = level

    def set_psi_is_sampled(self, is_sampled=True):
        """ Switch on/off the sampling of psi particles """
        self.psi_is_sampled = is_sampled

    def set_sigma_is_sampled(self, is_sampled=True):
        """ Switch on/off the sampling of sigma particles """
        self.sigma_is_sampled = is_sampled


    def create_sigma(self, name):
        """ This is called internally. Creates a nuisance
        on the structural uncertainty """
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
        """ This is called internally. Creates a nuisance
        on the data uncertainty """
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

    def set_label(self, s):
        """ Set the restraint output label """
        self.label=s

    def get_output(self):
        """ Get the output of the restraint to be used by the IMP.pmi.output object"""
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
                   xl_label] = str(-log(ln.unprotected_evaluate(None)))

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
        """ Get the particles to be sampled by the IMP.pmi.sampler object """
        ps = {}
        if self.sigma_is_sampled:
            for sigmaname in self.sigma_dictionary:
                ps["Nuisances_CrossLinkingMassSpectrometryRestraint_Sigma_" + str(sigmaname) + "_" + self.label] =\
                    ([self.sigma_dictionary[sigmaname][0]],
                     self.sigma_dictionary[sigmaname][1])

        if self.psi_is_sampled:
            for psiname in self.psi_dictionary:
                ps["Nuisances_CrossLinkingMassSpectrometryRestraint_Psi_" +
                    str(psiname) + "_" + self.label] =\
                   ([self.psi_dictionary[psiname][0]], self.psi_dictionary[psiname][1])

        return ps


class AtomicCrossLinkMSRestraint(object):
    """Setup cross-link distance restraints at atomic level
    The "atomic" aspect is that it models the particle uncertainty with a Gaussian
    The noise in the data and the structural uncertainty of cross-linked amino-acids
    is inferred using Bayes' theory of probabilty
    \note Wraps an IMP::isd::AtomicCrossLinkMSRestraint
    """
    def __init__(self,
                 root_hier,
                 xldb,
                 atom_type="NZ",
                 length=10.0,
                 slope=0.01,
                 nstates=None,
                 label='',
                 nuisances_are_optimized=True,
                 sigma_init=5.0,
                 psi_init = 0.01,
                 one_psi=True,
                 filelabel=None):
        """Constructor.
        Automatically creates one "sigma" per crosslinked residue and one "psis" per pair.
        Other nuisance options are available.
        \note Will return an error if the data+extra_sel don't specify two particles per XL pair.
        @param root_hier  The root hierarchy on which you'll do selection
        @param xldb       CrossLinkDataBase object
        @param atom_type  Can either be "NZ" or "CA"
        @param length     The XL linker length
        @param nstates    The number of states to model. Defaults to the number of states in root.
        @param label      The output label for the restraint
        @param nuisances_are_optimized Whether to optimize nuisances
        @param sigma_init The initial value for all the sigmas
        @param psi_init   The initial value for all the psis
        @param one_psi    Use a single psi for all restraints (if False, creates one per XL)
        @param filelabel automaticlly generated file containing missing/included/excluded
                cross-links will be labeled using this text
        """

        # basic params
        self.root = root_hier
        self.mdl = self.root.get_model()
        self.CrossLinkDataBase = xldb
        self.weight = 1.0
        self.label = label
        self.length = length
        self.sigma_is_sampled = nuisances_are_optimized
        self.psi_is_sampled = nuisances_are_optimized

        if atom_type not in("NZ","CA"):
            raise Exception("AtomicXLRestraint: atom_type must be NZ or CA")
        self.atom_type = atom_type
        self.nstates = nstates
        if nstates is None:
            self.nstates = len(IMP.atom.get_by_type(self.root,IMP.atom.STATE_TYPE))
        elif nstates!=len(IMP.atom.get_by_type(self.root,IMP.atom.STATE_TYPE)):
            print("Warning: nstates is not the same as the number of states in root")

        self.rs = IMP.RestraintSet(self.mdl, 'likelihood')
        self.rs_psi = IMP.RestraintSet(self.mdl, 'prior_psi')
        self.rs_sig = IMP.RestraintSet(self.mdl, 'prior_sigmas')
        self.rs_lin = IMP.RestraintSet(self.mdl, 'linear_dummy_restraints')


        self.psi_dictionary = {}
        self.sigma_dictionary = {}

        self.particles=defaultdict(set)
        self.one_psi = one_psi
        self.bonded_pairs = []
        if self.one_psi:
            print('creating a single psi for all XLs')
        else:
            print('creating one psi for each XL')

        # output logging file
        if filelabel is not None:
            indb = open("included." + filelabel + ".xl.db", "w")
            exdb = open("excluded." + filelabel + ".xl.db", "w")
            midb = open("missing." + filelabel + ".xl.db", "w")



        ### Setup nuisances
        '''
        # read ahead to get the number of XL's per residue
        num_xls_per_res=defaultdict(int)
        for unique_id in data:
           for nstate in range(self.nstates):
               for xl in data[unique_id]:
                   num_xls_per_res[str(xl.r1)]+=1
                   num_xls_per_res[str(xl.r2)]+=1

        # Will setup two sigmas based on promiscuity of the residue
        sig_threshold=4
        self.sig_low = setup_nuisance(self.mdl,self.rs_nuis,init_val=sigma_init,min_val=1.0,
                                      max_val=100.0,is_opt=self.nuis_opt)
        self.sig_high = setup_nuisance(self.mdl,self.rs_nuis,init_val=sigma_init,min_val=1.0,
                                       max_val=100.0,is_opt=self.nuis_opt)
        '''
        self._create_sigma('sigma',sigma_init)
        if one_psi:
            self._create_psi('psi',psi_init)
        else:
            for xlid in self.CrossLinkDataBase.xlid_iterator():
                self._create_psi(xlid,psi_init)

        ### create all the XLs
        xlrs=[]
        for xlid in self.CrossLinkDataBase.xlid_iterator():
            # create restraint for this data point
            if one_psi:
                psip = self.psi_dictionary['psi'][0].get_particle_index()
            else:
                psip = self.psi_dictionary[unique_id][0].get_particle_index()
            r = IMP.isd.AtomicCrossLinkMSRestraint(self.mdl,
                                                   self.length,
                                                   psip,
                                                   slope,
                                                   True)
            num_contributions=0

            # add a contribution for each XL ambiguity option within each state
            for nstate in self.nstates:
                for xl in self.CrossLinkDataBase[xlid]:
                    r1 = xl[self.CrossLinkDataBase.residue1_key]
                    c1 = xl[self.CrossLinkDataBase.protein1_key].strip()
                    r2 = xl[self.CrossLinkDataBase.residue2_key]
                    c2 = xl[self.CrossLinkDataBase.protein2_key].strip()

                    # perform selection. these may contain multiples if Copies are used
                    ps1 = IMP.atom.Selection(self.root,
                                             state_index=nstate,
                                             atom_type = IMP.atom.AtomType(self.atom_type),
                                             molecule=c1,
                                             residue_index=r1).get_selected_particles()
                    ps2 = IMP.atom.Selection(self.root,
                                             state_index=nstate,
                                             atom_type = IMP.atom.AtomType(self.atom_type),
                                             molecule=c2,
                                             residue_index=r2).get_selected_particles()
                    if len(ps1) == 0:
                        print("AtomicXLRestraint: WARNING> residue %d of chain %s is not there" % (r1, c1))
                        if filelabel is not None:
                            midb.write(str(xl) + "\n")
                        continue

                    if len(ps2) == 0:
                        print("AtomicXLRestraint: WARNING> residue %d of chain %s is not there" % (r2, c2))
                        if filelabel is not None:
                            midb.write(str(xl) + "\n")
                        continue


                    # figure out sig1 and sig2 based on num XLs
                    '''
                    num1=num_xls_per_res[str(xl.r1)]
                    num2=num_xls_per_res[str(xl.r2)]
                    if num1<sig_threshold:
                        sig1=self.sig_low
                    else:
                        sig1=self.sig_high
                    if num2<sig_threshold:
                        sig2=self.sig_low
                    else:
                        sig2=self.sig_high
                    '''
                    sig1 = self.sigma_dictionary['sigma'][0]
                    sig2 = self.sigma_dictionary['sigma'][0]

                    # add each copy contribution to restraint
                    for p1,p2 in itertools.product(ps1,ps2):
                        if p1==p2:
                            continue
                        self.particles[nstate]|=set((p1,p2))
                        r.add_contribution([p1.get_index(),p2.get_index()],
                                           [sig1.get_particle_index(),sig2.get_particle_index()])
                        num_contributions+=1

            if num_contributions>0:
                print('XL:',xlid,'num contributions:',num_contributions)
                xlrs.append(r)
        if len(xlrs)==0:
            raise Exception("You didn't create any XL restraints")
        print('created',len(xlrs),'XL restraints')
        self.rs=IMP.isd.LogWrapper(xlrs,self.weight)

    def set_weight(self,weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.rs)
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.rs_sig)
        IMP.pmi.tools.add_restraint_to_model(self.mdl, self.rs_psi)

    def get_hierarchy(self):
        return self.prot

    def get_restraint_set(self):
        return self.rs

    def _create_sigma(self, name,sigmainit):
        """ This is called internally. Creates a nuisance
        on the structural uncertainty """
        if name in self.sigma_dictionary:
            return self.sigma_dictionary[name][0]

        sigmaminnuis = 0.0000001
        sigmamaxnuis = 1000.0
        sigmamin = 0.01
        sigmamax = 100.0
        sigmatrans = 0.5
        sigma = IMP.pmi.tools.SetupNuisance(self.mdl,
                                            sigmainit,
                                            sigmaminnuis,
                                            sigmamaxnuis,
                                            self.sigma_is_sampled).get_particle()
        self.sigma_dictionary[name] = (
            sigma,
            sigmatrans,
            self.sigma_is_sampled)
        self.rs_sig.add_restraint(
            IMP.isd.UniformPrior(
                self.mdl,
                sigma,
                1000000000.0,
                sigmamax,
                sigmamin))
        return sigma

    def _create_psi(self, name,psiinit):
        """ This is called internally. Creates a nuisance
        on the data uncertainty """
        if name in self.psi_dictionary:
            return self.psi_dictionary[name][0]

        psiminnuis = 0.0000001
        psimaxnuis = 0.4999999
        psimin = 0.01
        psimax = 0.49
        psitrans = 0.1
        psi = IMP.pmi.tools.SetupNuisance(self.mdl,
                                          psiinit,
                                          psiminnuis,
                                          psimaxnuis,
                                          self.psi_is_sampled).get_particle()
        self.psi_dictionary[name] = (
            psi,
            psitrans,
            self.psi_is_sampled)

        self.rs_psi.add_restraint(
            IMP.isd.UniformPrior(
                self.mdl,
                psi,
                1000000000.0,
                psimax,
                psimin))

        self.rs_psi.add_restraint(IMP.isd.JeffreysRestraint(self.mdl, psi))
        return psi


    def create_restraints_for_rmf(self):
        """ create dummy harmonic restraints for each XL but don't add to model
        Makes it easy to see each contribution to each XL in RMF
        """
        class MyGetRestraint(object):
            def __init__(self,rs):
                self.rs=rs
            def get_restraint_for_rmf(self):
                return self.rs

        dummy_mdl=IMP.Model()
        hps = IMP.core.HarmonicDistancePairScore(self.length,1.0)
        dummy_rs=[]
        for nxl in range(self.get_number_of_restraints()):
            xl=IMP.isd.AtomicCrossLinkMSRestraint.get_from(self.rs.get_restraint(nxl))
            rs = IMP.RestraintSet(dummy_mdl, 'atomic_xl_'+str(nxl))
            for ncontr in range(xl.get_number_of_contributions()):
                ps=xl.get_contribution(ncontr)
                dr = IMP.core.PairRestraint(hps,[self.mdl.get_particle(p) for p in ps],
                                            'xl%i_contr%i'%(nxl,ncontr))
                rs.add_restraint(dr)
                dummy_rs.append(MyGetRestraint(rs))
        return dummy_rs


    def get_particles_to_sample(self,state_num=None):
        """ Get particles involved in the restraint """
        if state_num is None:
            return list(reduce(lambda x,y: self.particles[x]|self.particles[y],self.particles))
        else:
            return list(self.particles[state_num])

    def get_bonded_pairs(self):
        return self.bonded_pairs

    def get_number_of_restraints(self):
        return self.rs.get_number_of_restraints()

    def __repr__(self):
        return 'XL restraint with '+str(len(self.rs.get_restraint(0).get_number_of_restraints())) \
            + ' data points'

    def load_nuisances_from_stat_file(self,in_fn,nframe):
        """Read a stat file and load all the sigmas.
        This is potentially quite stupid.
        It's also a hack since the sigmas should be stored in the RMF file.
        Also, requires one sigma and one psi for ALL XLs.
        """
        import subprocess
        sig_val = float(subprocess.check_output(["process_output.py","-f",in_fn,
                                                 "-s","AtomicXLRestraint_sigma"]).split('\n>')[1+nframe])
        psi_val = float(subprocess.check_output(["process_output.py","-f",in_fn,
                                                 "-s","AtomicXLRestraint_psi"]).split('\n>')[1+nframe])
        for nxl in range(self.rs.get_number_of_restraints()):
            xl=IMP.isd.AtomicCrossLinkMSRestraint.get_from(self.rs.get_restraint(nxl))
            psip = xl.get_psi()
            IMP.isd.Scale(self.mdl,psip).set_scale(psi_val)
            for contr in range(xl.get_number_of_contributions()):
                sig1,sig2=xl.get_contribution_sigmas(contr)
                IMP.isd.Scale(self.mdl,sig1).set_scale(sig_val)

        print('loaded nuisances from file')

    def plot_violations(self,out_prefix,
                        max_prob_for_violation=0.1,
                        min_dist_for_violation=1e9,
                        coarsen=False,
                        limit_to_chains=None,
                        exclude_chains=''):
        """Create CMM files, one for each state, of all xinks.
        will draw in GREEN if non-violated in all states (or if only one state)
        will draw in PURPLE if non-violated only in a subset of states (draws nothing elsewhere)
        will draw in RED in ALL states if all violated
        (if only one state, you'll only see green and red)

        @param out_prefix             Output xlink files prefix
        @param max_prob_for_violation It's a violation if the probability is below this
        @param min_dist_for_violation It's a violation if the min dist is above this
        @param coarsen                Use CA positions
        @param limit_to_chains        Try to visualize just these chains
        @param exclude_to_chains        Try to NOT visualize these chains
        """
        print('going to calculate violations and plot CMM files')
        all_stats = self.get_best_stats()
        all_dists = [s["low_dist"] for s in all_stats]

        # prepare one output file per state
        out_fns=[]
        out_nvs=[]
        state_info=[]
        cmds = defaultdict(set)
        for nstate in range(self.nstates):
            outf=open(out_prefix+str(nstate)+'.cmm','w')
            outf.write('<marker_set name="xlinks_state%i"> \n' % nstate)
            out_fns.append(outf)
            out_nvs.append(0)
            print('will limit to',limit_to_chains)
            print('will exclude',exclude_chains)
            state_info.append(self.get_best_stats(nstate,
                                                  limit_to_chains,
                                                  exclude_chains))

        for nxl in range(self.rs.get_number_of_restraints()):
            # for this XL, check which states passed
            npass=[]
            nviol=[]
            for nstate in range(self.nstates):
                prob = state_info[nstate][nxl]["prob"]
                low_dist = state_info[nstate][nxl]["low_dist"]
                if prob<max_prob_for_violation or low_dist>min_dist_for_violation:
                    nviol.append(nstate)
                else:
                    npass.append(nstate)

            # special coloring when all pass or all fail
            all_pass=False
            all_viol=False
            if len(npass)==self.nstates:
                all_pass=True
            elif len(nviol)==self.nstates:
                all_viol=True

            # finally, color based on above info
            print(nxl,'state dists:',[state_info[nstate][nxl]["low_dist"] for nstate in range(self.nstates)],
                  'viol states:',nviol,'all viol?',all_viol)
            for nstate in range(self.nstates):
                if all_pass:
                    r=0.365; g=0.933; b=0.365;
                    continue
                elif all_viol:
                    r=0.980; g=0.302; b=0.247;
                    continue
                else:
                    if nstate in nviol:
                        continue
                    else:
                        #r=0.9; g=0.34; b=0.9;
                        r=0.365; g=0.933; b=0.365;
                # now only showing if UNIQUELY PASSING in this state
                pp = state_info[nstate][nxl]["low_pp"]
                c1=IMP.core.XYZ(self.mdl,pp[0]).get_coordinates()
                c2=IMP.core.XYZ(self.mdl,pp[1]).get_coordinates()

                r1 = IMP.atom.get_residue(IMP.atom.Atom(self.mdl,pp[0])).get_index()
                ch1 = IMP.atom.get_chain_id(IMP.atom.Atom(self.mdl,pp[0]))
                r2 = IMP.atom.get_residue(IMP.atom.Atom(self.mdl,pp[0])).get_index()
                ch2 = IMP.atom.get_chain_id(IMP.atom.Atom(self.mdl,pp[0]))

                cmds[nstate].add((ch1,r1))
                cmds[nstate].add((ch2,r2))

                outf = out_fns[nstate]
                nv = out_nvs[nstate]
                outf.write('<marker id= "%d" x="%.3f" y="%.3f" z="%.3f" radius="0.8" '
                           'r="%.2f" g="%.2f" b="%.2f"/> \n' % (nv,c1[0],c1[1],c1[2],r,g,b))
                outf.write('<marker id= "%d" x="%.3f" y="%.3f" z="%.3f" radius="0.8"  '
                           'r="%.2f" g="%.2f" b="%.2f"/> \n' % (nv+1,c2[0],c2[1],c2[2],r,g,b))
                outf.write('<link id1= "%d" id2="%d" radius="0.8" '
                           'r="%.2f" g="%.2f" b="%.2f"/> \n' % (nv,nv+1,r,g,b))
                out_nvs[nstate]+=2

        for nstate in range(self.nstates):
            out_fns[nstate].write('</marker_set>\n')
            out_fns[nstate].close()
            cmd = ''
            for ch,r in cmds[nstate]:
                cmd+='#%i:%i.%s '%(nstate,r,ch)
            print(cmd)

        return all_dists
    def _get_contribution_info(self,xl,ncontr,use_CA=False):
        """Return the particles at that contribution. If requested will return CA's instead"""
        idx1=xl.get_contribution(ncontr)[0]
        idx2=xl.get_contribution(ncontr)[1]
        if use_CA:
            idx1 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.mdl,idx1)),
                                      atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
            idx2 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.mdl,idx2)),
                                      atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
        dist = IMP.algebra.get_distance(IMP.core.XYZ(self.mdl,idx1).get_coordinates(),
                                        IMP.core.XYZ(self.mdl,idx2).get_coordinates())
        return idx1,idx2,dist

    def get_best_stats(self,limit_to_state=None,limit_to_chains=None,exclude_chains='',use_CA=False):
        ''' return the probability, best distance, two coords, and possibly the psi for each xl
        @param limit_to_state Only examine contributions from one state
        @param limit_to_chains Returns the particles for certain "easy to visualize" chains
        @param exclude_chains  Even if you limit, don't let one end be in this list.
                               Only works if you also limit chains
        @param use_CA          Limit to CA particles
        '''
        ret = []
        for nxl in range(self.rs.get_number_of_restraints()):
            this_info = {}
            xl=IMP.isd.AtomicCrossLinkMSRestraint.get_from(self.rs.get_restraint(nxl))
            low_dist=1e6
            low_contr = None
            low_pp = None
            state_contrs=[]
            low_pp_lim = None
            low_dist_lim = 1e6
            for contr in range(xl.get_number_of_contributions()):
                pp = xl.get_contribution(contr)
                if use_CA:
                    idx1 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.mdl,pp[0])),
                                              atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
                    idx2 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.mdl,pp[1])),
                                              atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
                    pp = [idx1,idx2]
                if limit_to_state is not None:
                    nstate = IMP.atom.get_state_index(IMP.atom.Atom(self.mdl,pp[0]))
                    if nstate!=limit_to_state:
                        continue
                    state_contrs.append(contr)
                dist = IMP.core.get_distance(IMP.core.XYZ(self.mdl,pp[0]),
                                             IMP.core.XYZ(self.mdl,pp[1]))
                if limit_to_chains is not None:
                    c1 = IMP.atom.get_chain_id(IMP.atom.Atom(self.mdl,pp[0]))
                    c2 = IMP.atom.get_chain_id(IMP.atom.Atom(self.mdl,pp[1]))
                    if (c1 in limit_to_chains or c2 in limit_to_chains) and (
                            c1 not in exclude_chains and c2 not in exclude_chains):
                        if dist<low_dist_lim:
                            low_dist_lim = dist
                            low_pp_lim = pp
                if dist<low_dist:
                    low_dist = dist
                    low_contr = contr
                    low_pp = pp
            if limit_to_state is not None:
                this_info["prob"] = xl.evaluate_for_contributions(state_contrs,None)
            else:
                this_info["prob"] = xl.unprotected_evaluate(None)
            if limit_to_chains is not None:
                this_info["low_pp"] = low_pp_lim
            else:
                this_info["low_pp"] = low_pp

            this_info["low_dist"] = low_dist
            if not self.one_psi:
                pval = IMP.isd.Scale(self.mdl,xl.get_psi()).get_scale()
                this_info["psi"] = pval
            ret.append(this_info)
        return ret

    def print_stats(self):
        #print("XL restraint statistics\n<num> <prob> <bestdist> <sig1> <sig2> <is_viol>")
        stats = self.get_best_stats()
        for nxl,s in enumerate(stats):
            #print('%i %.4f %.4f %.4f %.4f %i'%(nxl,prob,low_dist,sig1,sig2,is_viol))
            print(s["low_dist"])


    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["AtomicXLRestraint" + self.label] = str(score)

        ### HACK to make it easier to see the few sigmas
        #output["AtomicXLRestraint_sigma"] = self.sigma.get_scale()
        output["AtomicXLRestraint_sigma"] = self.rs_sig.unprotected_evaluate(None)
        output["AtomicXLRestraint_psi"] = self.rs_psi.unprotected_evaluate(None)
        #if self.one_psi:
        #    output["AtomicXLRestraint_psi"] = self.psi.get_scale()
        ######

        # count distances above length
        bad_count=0
        stats = self.get_best_stats()
        for nxl,s in enumerate(stats):
            if s['low_dist']>20.0:
                bad_count+=1
            output["AtomicXLRestraint_%i_%s"%(nxl,"Prob")]=str(s['prob'])
            output["AtomicXLRestraint_%i_%s"%(nxl,"BestDist")]=str(s['low_dist'])
            if not self.one_psi:
                output["AtomicXLRestraint_%i_%s"%(nxl,"psi")]=str(s['psi'])
        output["AtomicXLRestraint_NumViol"] = str(bad_count)
        return output


@IMP.deprecated_object("2.5", "Use IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint instead.")
class ConnectivityCrossLinkMS(object):
    """This restraint allows ambiguous crosslinking between multiple copies
    it is a variant of the SimplifiedCrossLinkMS
    """
    def __init__(
        self,
        representation,
        restraints_file,
        expdistance,
        strength=None,
            resolution=None):

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.weight = 1.0
        self.label = "None"
        self.pairs = []

        self.outputlevel = "low"
        self.expdistance = expdistance
        self.strength = strength

        fl = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        for line in fl:

            tokens = line.split()
            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[2])
            c1 = tokens[0]
            r2 = int(tokens[3])
            c2 = tokens[1]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=True,
                residue=r1)
            hrc1 = [representation.hier_db.particle_to_name[p] for p in ps1]
            if len(ps1) == 0:
                print("ConnectivityCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue

            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=True,
                residue=r2)
            hrc2 = [representation.hier_db.particle_to_name[p] for p in ps2]
            if len(ps2) == 0:
                print("ConnectivityCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            s1 = IMP.atom.Selection(ps1)
            s2 = IMP.atom.Selection(ps2)

            # calculate the radii to estimate the slope of the restraint
            if self.strength is None:
                rad1 = 0
                rad2 = 0
                for p in ps1:
                    rad1 += IMP.pmi.Uncertainty(p).get_uncertainty()

                for p in ps2:
                    rad2 += IMP.pmi.Uncertainty(p).get_uncertainty()

                rad1 = rad1 / len(ps1)
                rad2 = rad2 / len(ps2)

                self.strength = 1 / (rad1 ** 2 + rad2 ** 2)

            sels = [s1, s2]
            cr = IMP.atom.create_connectivity_restraint(
                sels,
                self.expdistance,
                self.strength)

            self.rs.add_restraint(cr)
            self.pairs.append((ps1, hrc1, c1, r1, ps2, hrc2, c2, r2, cr))

    def plot_restraint(
        self,
        uncertainty1,
        uncertainty2,
        maxdist=50,
            npoints=10):

        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        d1.set_radius(uncertainty1)
        d2.set_radius(uncertainty2)
        s1 = IMP.atom.Selection(p1)
        s2 = IMP.atom.Selection(p2)
        sels = [s1, s2]
        strength = 1 / (uncertainty1 ** 2 + uncertainty2 ** 2)
        cr = IMP.atom.create_connectivity_restraint(
            sels,
            self.expdistance,
            strength)
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(cr.unprotected_evaluate(None))
        IMP.pmi.output.plot_xy_data(dists, scores)

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

    def set_output_level(self, level="low"):
            # this might be "low" or "high"
        self.outputlevel = level

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ConnectivityCrossLinkMS_Score_" + self.label] = str(score)
        for n, p in enumerate(self.pairs):

            ps1 = p[0]
            hrc1 = p[1]
            c1 = p[2]
            r1 = p[3]
            ps2 = p[4]
            hrc2 = p[5]
            c2 = p[6]
            r2 = p[7]
            cr = p[8]
            for n1, p1 in enumerate(ps1):
                name1 = hrc1[n1]

                for n2, p2 in enumerate(ps2):
                    name2 = hrc2[n2]
                    d1 = IMP.core.XYZR(p1)
                    d2 = IMP.core.XYZR(p2)
                    label = str(r1) + ":" + name1 + "_" + str(r2) + ":" + name2
                    output["ConnectivityCrossLinkMS_Distance_" +
                           label] = str(IMP.core.get_distance(d1, d2))

            label = str(r1) + ":" + c1 + "_" + str(r2) + ":" + c2
            output["ConnectivityCrossLinkMS_Score_" +
                   label] = str(self.weight * cr.unprotected_evaluate(None))

        return output

@IMP.deprecated_object("2.5", "Use IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint instead.")
class SimplifiedCrossLinkMS(object):
    def __init__(
        self,
        representation,
        restraints_file,
        expdistance,
        strength,
        resolution=None,
        columnmapping=None,
        truncated=True,
            spheredistancepairscore=True):
        # columnindexes is a list of column indexes for protein1, protein2, residue1, residue2
        # by default column 0 = protein1; column 1 = protein2; column 2 =
        # residue1; column 3 = residue2

        if columnmapping is None:
            columnmapping = {}
            columnmapping["Protein1"] = 0
            columnmapping["Protein2"] = 1
            columnmapping["Residue1"] = 2
            columnmapping["Residue2"] = 3

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.weight = 1.0
        self.label = "None"
        self.pairs = []
        self.already_added_pairs = {}

        self.outputlevel = "low"
        self.expdistance = expdistance
        self.strength = strength
        self.truncated = truncated
        self.spheredistancepairscore = spheredistancepairscore

        # fill the cross-linker pmfs
        # to accelerate the init the list listofxlinkertypes might contain only
        # yht needed crosslinks
        protein1 = columnmapping["Protein1"]
        protein2 = columnmapping["Protein2"]
        residue1 = columnmapping["Residue1"]
        residue2 = columnmapping["Residue2"]

        fl = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        for line in fl:

            tokens = line.split()
            # skip character
            if (tokens[0] == "#"):
                continue
            r1 = int(tokens[residue1])
            c1 = tokens[protein1]
            r2 = int(tokens[residue2])
            c2 = tokens[protein2]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=False,
                residue=r1)
            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=False,
                residue=r2)

            if len(ps1) > 1:
                raise ValueError(
                   "residue %d of chain %s selects multiple particles; "
                   "particles are: %s"
                   % (r1, c1, "".join(p.get_name() for p in ps1)))
            elif len(ps1) == 0:
                print("SimplifiedCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                continue

            if len(ps2) > 1:
                raise ValueError(
                   "residue %d of chain %s selects multiple particles; "
                   "particles are: %s"
                   % (r2, c2, "".join(p.get_name() for p in ps2)))
            elif len(ps2) == 0:
                print("SimplifiedCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                continue

            p1 = ps1[0]
            p2 = ps2[0]

            if (p1, p2) in self.already_added_pairs:
                dr = self.already_added_pairs[(p1, p2)]
                weight = dr.get_weight()
                dr.set_weight(weight + 1.0)
                print("SimplifiedCrossLinkMS> crosslink %d %s %d %s was already found, adding 1.0 to the weight, weight is now %d" % (r1, c1, r2, c2, weight + 1.0))
                continue

            else:

                if self.truncated:
                    limit = self.strength * (self.expdistance + 15) ** 2 + 10.0
                    hub = IMP.core.TruncatedHarmonicUpperBound(
                        self.expdistance,
                        self.strength,
                        self.expdistance +
                        15.,
                        limit)
                else:
                    hub = IMP.core.HarmonicUpperBound(
                        self.expdistance,
                        self.strength)
                if self.spheredistancepairscore:
                    df = IMP.core.SphereDistancePairScore(hub)
                else:
                    df = IMP.core.DistancePairScore(hub)
                dr = IMP.core.PairRestraint(df, (p1, p2))
                dr.set_name(c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2))

                self.rs.add_restraint(dr)
                self.pairs.append((p1, p2, dr, r1, c1, r2, c2))
                self.already_added_pairs[(p1, p2)] = dr
                self.already_added_pairs[(p2, p1)] = dr

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

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

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def plot_restraint(self, radius1, radius2, maxdist=50, npoints=10):

        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        d1.set_radius(radius1)
        d2.set_radius(radius2)
        if self.truncated:
            limit = self.strength * (self.expdistance + 15) ** 2 + 10.0
            hub = IMP.core.TruncatedHarmonicUpperBound(
                self.expdistance,
                self.strength,
                self.expdistance +
                15.,
                limit)
        else:
            hub = IMP.core.HarmonicUpperBound(
                self.expdistance,
                self.strength)
        df = IMP.core.SphereDistancePairScore(hub)
        dr = IMP.core.PairRestraint(df, (p1, p2))
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(dr.unprotected_evaluate(None))
        IMP.pmi.output.plot_xy_data(dists, scores)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["SimplifiedCrossLinkMS_Score_" + self.label] = str(score)
        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            crosslinker = 'standard'
            ln = self.pairs[i][2]
            resid1 = self.pairs[i][3]
            chain1 = self.pairs[i][4]
            resid2 = self.pairs[i][5]
            chain2 = self.pairs[i][6]

            label = str(resid1) + ":" + chain1 + "_" + \
                str(resid2) + ":" + chain2
            output["SimplifiedCrossLinkMS_Score_" + crosslinker + "_" +
                   label] = str(self.weight * ln.unprotected_evaluate(None))

            if self.spheredistancepairscore:
                d0 = IMP.core.XYZR(p0)
                d1 = IMP.core.XYZR(p1)
            else:
                d0 = IMP.core.XYZ(p0)
                d1 = IMP.core.XYZ(p1)
            output["SimplifiedCrossLinkMS_Distance_" +
                   label] = str(IMP.core.get_distance(d0, d1))

        return output

#

@IMP.deprecated_object("2.5", "Use IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint instead.")
class SigmoidalCrossLinkMS(object):
    def __init__(
        self, representation, restraints_file, inflection, slope, amplitude,
        linear_slope, resolution=None, columnmapping=None, csvfile=False,
            filters=None, filelabel="None"):
        # columnindexes is a list of column indexes for protein1, protein2, residue1, residue2
        # by default column 0 = protein1; column 1 = protein2; column 2 = residue1; column 3 = residue2
        # the filters applies to the csvfile, the format is
        # filters=[("Field1",">|<|=|>=|<=",value),("Field2","is","String"),("Field2","in","String")]

        if columnmapping is None:
            columnmapping = {}
            columnmapping["Protein1"] = 0
            columnmapping["Protein2"] = 1
            columnmapping["Residue1"] = 2
            columnmapping["Residue2"] = 3

        if csvfile:
            # in case the file is a cvs file
            # columnmapping will contain the field names
            # that compare in the first line of the cvs file
            db = tools.get_db_from_csv(restraints_file)
        else:
            db = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        self.m = representation.prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.weight = 1.0

        self.label = "None"
        self.pairs = []
        self.already_added_pairs = {}
        self.inflection = inflection
        self.slope = slope
        self.amplitude = amplitude
        self.linear_slope = linear_slope

        self.outputlevel = "low"

        # fill the cross-linker pmfs
        # to accelerate the init the list listofxlinkertypes might contain only
        # yht needed crosslinks
        protein1 = columnmapping["Protein1"]
        protein2 = columnmapping["Protein2"]
        residue1 = columnmapping["Residue1"]
        residue2 = columnmapping["Residue2"]

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")

        for entry in db:
            if not csvfile:
                tokens = entry.split()
                # skip character
                if (tokens[0] == "#"):
                    continue
                r1 = int(tokens[residue1])
                c1 = tokens[protein1]
                r2 = int(tokens[residue2])
                c2 = tokens[protein2]
            else:
                if filters is not None:
                    if eval(tools.cross_link_db_filter_parser(filters)) == False:
                        exdb.write(str(entry) + "\n")
                        continue
                indb.write(str(entry) + "\n")
                r1 = int(entry[residue1])
                c1 = entry[protein1]
                r2 = int(entry[residue2])
                c2 = entry[protein2]

            ps1 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c1,
                name_is_ambiguous=False,
                residue=r1)
            ps2 = IMP.pmi.tools.select(
                representation,
                resolution=resolution,
                name=c2,
                name_is_ambiguous=False,
                residue=r2)

            if len(ps1) > 1:
                raise ValueError("residue %d of chain %s selects multiple particles %s" % (r1, c1, str(ps1)))
            elif len(ps1) == 0:
                print("SigmoidalCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                midb.write(str(entry) + "\n")
                continue

            if len(ps2) > 1:
                raise ValueError("residue %d of chain %s selects multiple particles %s" % (r2, c2, str(ps2)))
            elif len(ps2) == 0:
                print("SigmoidalCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                midb.write(str(entry) + "\n")
                continue

            p1 = ps1[0]
            p2 = ps2[0]

            if (p1, p2) in self.already_added_pairs:
                dr = self.already_added_pairs[(p1, p2)]
                weight = dr.get_weight()
                dr.increment_amplitude(amplitude)
                print("SigmoidalCrossLinkMS> crosslink %d %s %d %s was already found, adding %d to the amplitude, amplitude is now %d" % (r1, c1, r2, c2, amplitude, dr.get_amplitude()))
                dr.set_name(c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2)
                            + "-ampl:" + str(dr.get_amplitude()))
                continue

            else:

                dr = IMP.pmi.SigmoidRestraintSphere(
                    self.m,
                    p1,
                    p2,
                    self.inflection,
                    self.slope,
                    self.amplitude,
                    self.linear_slope)
                dr.set_name(c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2)
                            + "-ampl:" + str(dr.get_amplitude()))

                self.rs.add_restraint(dr)

                self.pairs.append((p1, p2, dr, r1, c1, r2, c2))
                self.already_added_pairs[(p1, p2)] = dr
                self.already_added_pairs[(p2, p1)] = dr

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_restraint_sets(self):
        return self.rs

    def get_restraint(self):
        return self.rs

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

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def plot_restraint(self, radius1, radius2, maxdist=50, npoints=10):
        p1 = IMP.Particle(self.m)
        p2 = IMP.Particle(self.m)
        d1 = IMP.core.XYZR.setup_particle(p1)
        d2 = IMP.core.XYZR.setup_particle(p2)
        d1.set_radius(radius1)
        d2.set_radius(radius2)
        dr = IMP.pmi.SigmoidRestraintSphere(
            self.m,
            p1,
            p2,
            self.inflection,
            self.slope,
            self.amplitude,
            self.linear_slope)
        dists = []
        scores = []
        for i in range(npoints):
            d2.set_coordinates(
                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0, 0))
            dists.append(IMP.core.get_distance(d1, d2))
            scores.append(dr.unprotected_evaluate(None))
        IMP.pmi.output.plot_xy_data(dists, scores)

    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["SigmoidalCrossLinkMS_Score_" + self.label] = str(score)
        for i in range(len(self.pairs)):

            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            crosslinker = 'standard'
            ln = self.pairs[i][2]
            resid1 = self.pairs[i][3]
            chain1 = self.pairs[i][4]
            resid2 = self.pairs[i][5]
            chain2 = self.pairs[i][6]

            label = str(resid1) + ":" + chain1 + "_" + \
                str(resid2) + ":" + chain2
            output["SigmoidalCrossLinkMS_Score_" + crosslinker + "_" +
                   label + "_" + self.label] = str(self.weight * ln.unprotected_evaluate(None))
            d0 = IMP.core.XYZR(p0)
            d1 = IMP.core.XYZR(p1)
            output["SigmoidalCrossLinkMS_Distance_" +
                   label + "_" + self.label] = str(IMP.core.get_distance(d0, d1))

        return output

@IMP.deprecated_object("2.5", "Use IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint instead.")
class ISDCrossLinkMS(IMP.pmi.restraints._NuisancesBase):
    def __init__(self, representation,
                 restraints_file,
                 length,
                 jackknifing=None,
                 # jackknifing (Float [0,1]) default=None; percentage of data to be
                 # removed randomly
                 resolution=None,
                 # resolution (Non-negative Integer) default=None; percentage of data
                 # to be removed randomly
                 slope=0.0,
                 inner_slope=0.0,
                 columnmapping=None,
                 rename_dict=None,
                 offset_dict=None,
                 csvfile=False,
                 ids_map=None,
                 radius_map=None,
                 filters=None,
                 label="None",
                 filelabel="None",
                 automatic_sigma_classification=False,
                 attributes_for_label=None):

        # columnindexes is a list of column indexes for protein1, protein2, residue1, residue2,idscore, XL unique id
        # by default column 0 = protein1; column 1 = protein2; column 2 = residue1; column 3 = residue2;
        # column 4 = idscores
        # attributes_for_label: anything in the csv database that must be added to the label
        # slope is the slope defined on the linear function
        # inner_slope is the slope defined on the restraint directly
        # suggestion: do not use both!

        if type(representation) != list:
            representations = [representation]
        else:
            representations = representation

        if columnmapping is None:
            columnmapping = {}
            columnmapping["Protein1"] = 0
            columnmapping["Protein2"] = 1
            columnmapping["Residue1"] = 2
            columnmapping["Residue2"] = 3
            columnmapping["IDScore"] = 4
            columnmapping["XLUniqueID"] = 5

        if csvfile:
            # in case the file is a cvs file
            # columnmapping will contain the field names
            # that compare in the first line of the cvs file
            db = IMP.pmi.tools.get_db_from_csv(restraints_file)
        else:
            db = IMP.pmi.tools.open_file_or_inline_text(restraints_file)

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")

        self.m = representations[0].prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'data')
        self.rspsi = IMP.RestraintSet(self.m, 'prior_psi')
        self.rssig = IMP.RestraintSet(self.m, 'prior_sigmas')
        self.rslin = IMP.RestraintSet(self.m, 'prior_linear')
        self.rslen = IMP.RestraintSet(self.m, 'prior_length')

        self.label = label
        self.pairs = []
        self.sigma_dictionary = {}
        self.psi_dictionary = {}
        self.psi_is_sampled = True
        self.sigma_is_sampled = True

        # isd_map is a dictionary/map that is used to determine the psi
        # parameter from identity scores (such as ID-Score, or FDR)
        if ids_map is None:
            self.ids_map = IMP.pmi.tools.map()
            self.ids_map.set_map_element(20.0, 0.05)
            self.ids_map.set_map_element(65.0, 0.01)
        else:
            self.ids_map = ids_map

        if radius_map is None:
            self.radius_map = IMP.pmi.tools.map()
            if automatic_sigma_classification:
                self.radius_map.set_map_element(10, 10)
            self.radius_map.set_map_element(1, 1)
        else:
            self.radius_map = radius_map

        self.outputlevel = "low"

        # small linear contribution for long range
        self.linear = IMP.core.Linear(0, 0.0)
        self.linear.set_slope(slope)
        dps2 = IMP.core.DistancePairScore(self.linear)

        protein1 = columnmapping["Protein1"]
        protein2 = columnmapping["Protein2"]
        residue1 = columnmapping["Residue1"]
        residue2 = columnmapping["Residue2"]
        idscore = columnmapping["IDScore"]
        try:
            xluniqueid = columnmapping["XLUniqueID"]
        except:
            xluniqueid = None

        restraints = []

        # we need this dictionary to create ambiguity (i.e., multistate)
        # if one id is already present in the dictionary, add the term to the
        # corresponding already generated restraint

        uniqueid_restraints_map = {}

        for nxl, entry in enumerate(db):

            if not jackknifing is None:

                # to be implemented
                # the problem is that in the replica exchange
                # you have to broadcast the same restraints to every
                # replica

                raise NotImplementedError("jackknifing not yet implemented")

            if not csvfile:
                tokens = entry.split()
                if len(tokens)==0:
                    continue

                # skip character
                if (tokens[0] == "#"):
                    continue
                try:
                    r1 = int(tokens[residue1])
                    c1 = tokens[protein1]
                    r2 = int(tokens[residue2])
                    c2 = tokens[protein2]

                    if offset_dict is not None:
                        if c1 in offset_dict: r1+=offset_dict[c1]
                        if c2 in offset_dict: r2+=offset_dict[c2]

                    if rename_dict is not None:
                        if c1 in rename_dict: c1=rename_dict[c1]
                        if c2 in rename_dict: c2=rename_dict[c2]

                    if idscore is None:
                        ids = 1.0
                    else:
                        ids = float(tokens[idscore])
                    if xluniqueid is None:
                        xlid = str(nxl)
                    else:
                        xlid = tokens[xluniqueid]
                except:
                    print("this line was not accessible " + str(entry))
                    if residue1 not in entry: print(str(residue1)+" keyword not in database")
                    if residue2 not in entry: print(str(residue2)+" keyword not in database")
                    if protein1 not in entry: print(str(protein1)+" keyword not in database")
                    if protein2 not in entry: print(str(protein2)+" keyword not in database")
                    if idscore not in entry: print(str(idscore)+" keyword not in database")
                    if xluniqueid not in entry: print(str(xluniqueid)+" keyword not in database")
                    continue

            else:
                if filters is not None:
                    if eval(IMP.pmi.tools.cross_link_db_filter_parser(filters)) == False:
                        exdb.write(str(entry) + "\n")
                        continue

                try:
                    r1 = int(entry[residue1])
                    c1 = entry[protein1]
                    r2 = int(entry[residue2])
                    c2 = entry[protein2]

                    if offset_dict is not None:
                        if c1 in offset_dict: r1+=offset_dict[c1]
                        if c2 in offset_dict: r2+=offset_dict[c2]

                    if rename_dict is not None:
                        if c1 in rename_dict: c1=rename_dict[c1]
                        if c2 in rename_dict: c2=rename_dict[c2]

                    if idscore is None:
                        ids = 1.0
                    else:
                        try:
                            ids = float(entry[idscore])
                        except ValueError:
                            ids = entry[idscore]
                    if xluniqueid is None:
                        xlid = str(nxl)
                    else:
                        xlid = entry[xluniqueid]

                except:
                    print("this line was not accessible " + str(entry))
                    if residue1 not in entry: print(str(residue1)+" keyword not in database")
                    if residue2 not in entry: print(str(residue2)+" keyword not in database")
                    if protein1 not in entry: print(str(protein1)+" keyword not in database")
                    if protein2 not in entry: print(str(protein2)+" keyword not in database")
                    if idscore not in entry: print(str(idscore)+" keyword not in database")
                    if xluniqueid not in entry: print(str(xluniqueid)+" keyword not in database")
                    continue

            for nstate, r in enumerate(representations):
                # loop over every state

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
                    print("ISDCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r1, c1))
                    midb.write(str(entry) + "\n")
                    continue

                if len(ps2) > 1:
                    raise ValueError("residue %d of chain %s selects multiple particles %s" % (r2, c2, str(ps2)))
                elif len(ps2) == 0:
                    print("ISDCrossLinkMS: WARNING> residue %d of chain %s is not there" % (r2, c2))
                    midb.write(str(entry) + "\n")
                    continue

                p1 = ps1[0]
                p2 = ps2[0]

                if (p1 == p2) and (r1 == r2) :
                    print("ISDCrossLinkMS Restraint: WARNING> on the identical bead particles and the identical residues, thus skipping this cross-link.")
                    continue

                if xlid in uniqueid_restraints_map:
                    print("Appending a crosslink restraint into the uniqueID %s" % str(xlid))
                    dr = uniqueid_restraints_map[xlid]
                else:
                    print("Generating a NEW crosslink restraint with a uniqueID %s" % str(xlid))
                    dr = IMP.isd.CrossLinkMSRestraint(
                        self.m,
                        length,
                        inner_slope)
                    restraints.append(dr)
                    uniqueid_restraints_map[xlid] = dr

                mappedr1 = self.radius_map.get_map_element(
                    IMP.pmi.Uncertainty(p1).get_uncertainty())
                sigma1 = self.get_sigma(mappedr1)[0]
                mappedr2 = self.radius_map.get_map_element(
                    IMP.pmi.Uncertainty(p2).get_uncertainty())
                sigma2 = self.get_sigma(mappedr2)[0]

                psival = self.ids_map.get_map_element(ids)
                psi = self.get_psi(psival)[0]


                p1i = p1.get_particle_index()
                p2i = p2.get_particle_index()
                s1i = sigma1.get_particle().get_index()
                s2i = sigma2.get_particle().get_index()

                #print nstate, p1i, p2i, p1.get_name(), p2.get_name()

                psii = psi.get_particle().get_index()

                dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
                print("--------------")
                print("ISDCrossLinkMS: generating cross-link restraint between")
                print("ISDCrossLinkMS: residue %d of chain %s and residue %d of chain %s" % (r1, c1, r2, c2))
                print("ISDCrossLinkMS: with sigma1 %f sigma2 %f psi %s" % (mappedr1, mappedr2, psival))
                print("ISDCrossLinkMS: between particles %s and %s" % (p1.get_name(), p2.get_name()))
                print("==========================================\n")
                indb.write(str(entry) + "\n")

                # check if the two residues belong to the same rigid body
                if(IMP.core.RigidMember.get_is_setup(p1) and
                   IMP.core.RigidMember.get_is_setup(p2) and
                   IMP.core.RigidMember(p1).get_rigid_body() ==
                   IMP.core.RigidMember(p2).get_rigid_body()):
                    xlattribute = "intrarb"
                else:
                    xlattribute = "interrb"

                if csvfile:
                    if not attributes_for_label is None:
                        for a in attributes_for_label:
                            xlattribute = xlattribute + "_" + str(entry[a])

                xlattribute = xlattribute + "-State:" + str(nstate)

                dr.set_name(
                    xlattribute + "-" + c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2) + "_" + self.label)

                if p1i != p2i:
                    pr = IMP.core.PairRestraint(self.m, dps2, (p1i, p2i))
                    pr.set_name(
                        xlattribute + "-" + c1 + ":" + str(r1) + "-" + c2 + ":" + str(r2) + "_" + self.label)
                    self.rslin.add_restraint(pr)


                self.pairs.append(
                    (p1,
                     p2,
                     dr,
                     r1,
                     c1,
                     r2,
                     c2,
                     xlattribute,
                     mappedr1,
                     mappedr2,
                     psival,
                     xlid,
                     nstate,
                     ids))

        lw = IMP.isd.LogWrapper(restraints,1.0)
        self.rs.add_restraint(lw)


    def set_slope_linear_term(self, slope):
        self.linear.set_slope(slope)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rspsi)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rssig)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rslen)
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

    def get_label(self,pairs_index):
        resid1 = self.pairs[pairs_index][3]
        chain1 = self.pairs[pairs_index][4]
        resid2 = self.pairs[pairs_index][5]
        chain2 = self.pairs[pairs_index][6]
        attribute = self.pairs[pairs_index][7]
        rad1 = self.pairs[pairs_index][8]
        rad2 = self.pairs[pairs_index][9]
        psi = self.pairs[pairs_index][10]
        xlid= self.pairs[pairs_index][11]
        label = attribute + "-" + \
            str(resid1) + ":" + chain1 + "_" + str(resid2) + ":" + \
            chain2 + "-" + str(rad1) + "-" + str(rad2) + "-" + str(psi)
        return label

    def write_db(self,filename):
        cldb=IMP.pmi.output.CrossLinkIdentifierDatabase()

        for pairs_index in range(len(self.pairs)):

            resid1 = self.pairs[pairs_index][3]
            chain1 = self.pairs[pairs_index][4]
            resid2 = self.pairs[pairs_index][5]
            chain2 = self.pairs[pairs_index][6]
            attribute = self.pairs[pairs_index][7]
            rad1 = self.pairs[pairs_index][8]
            rad2 = self.pairs[pairs_index][9]
            psi = self.pairs[pairs_index][10]
            xlid= self.pairs[pairs_index][11]
            nstate=self.pairs[pairs_index][12]
            ids=self.pairs[pairs_index][13]

            label=self.get_label(pairs_index)
            cldb.set_unique_id(label,xlid)
            cldb.set_protein1(label,chain1)
            cldb.set_protein2(label,chain2)
            cldb.set_residue1(label,resid1)
            cldb.set_residue2(label,resid2)
            cldb.set_idscore(label,ids)
            cldb.set_state(label,nstate)
            cldb.set_sigma1(label,rad1)
            cldb.set_sigma2(label,rad2)
            cldb.set_psi(label,psi)
            cldb.write(filename)


    def get_output(self):
        # content of the crosslink database pairs
        # self.pairs.append((p1,p2,dr,r1,c1,r2,c2))
        self.m.update()

        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["ISDCrossLinkMS_Data_Score_" + self.label] = str(score)
        output["ISDCrossLinkMS_PriorSig_Score_" +
               self.label] = self.rssig.unprotected_evaluate(None)
        output["ISDCrossLinkMS_PriorPsi_Score_" +
                   self.label] = self.rspsi.unprotected_evaluate(None)
        output["ISDCrossLinkMS_Linear_Score_" +
               self.label] = self.rslin.unprotected_evaluate(None)
        for i in range(len(self.pairs)):

            label=self.get_label(i)
            ln = self.pairs[i][2]
            p0 = self.pairs[i][0]
            p1 = self.pairs[i][1]
            output["ISDCrossLinkMS_Score_" +
                   label + "_" + self.label] = str(-log(ln.unprotected_evaluate(None)))

            d0 = IMP.core.XYZ(p0)
            d1 = IMP.core.XYZ(p1)
            output["ISDCrossLinkMS_Distance_" +
                   label + "_" + self.label] = str(IMP.core.get_distance(d0, d1))


        for psiindex in self.psi_dictionary:
            output["ISDCrossLinkMS_Psi_" +
                    str(psiindex) + "_" + self.label] = str(self.psi_dictionary[psiindex][0].get_scale())

        for resolution in self.sigma_dictionary:
            output["ISDCrossLinkMS_Sigma_" +
                   str(resolution) + "_" + self.label] = str(self.sigma_dictionary[resolution][0].get_scale())


        return output

    def get_particles_to_sample(self):
        ps = {}

        for resolution in self.sigma_dictionary:
            if self.sigma_dictionary[resolution][2] and self.sigma_is_sampled:
                ps["Nuisances_ISDCrossLinkMS_Sigma_" + str(resolution) + "_" + self.label] =\
                    ([self.sigma_dictionary[resolution][0]],
                     self.sigma_dictionary[resolution][1])

        if self.psi_is_sampled:
            for psiindex in self.psi_dictionary:
                if self.psi_dictionary[psiindex][2]:
                    ps["Nuisances_ISDCrossLinkMS_Psi_" +
                        str(psiindex) + "_" + self.label] = ([self.psi_dictionary[psiindex][0]], self.psi_dictionary[psiindex][1])

        return ps

#
class CysteineCrossLinkRestraint(object):
    def __init__(self, representations, filename, cbeta=False,
                 betatuple=(0.03, 0.1),
                 disttuple=(0.0, 25.0, 1000),
                 omegatuple=(1.0, 1000.0, 50),
                 sigmatuple=(0.3, 0.3, 1),
                 betaissampled=True,
                 sigmaissampled=False,
                 weightissampled=True,
                 epsilonissampled=True
                 ):
    # the file must have residue1 chain1 residue2 chain2 fractionvalue epsilonname
    # epsilonname is a name for the epsilon particle that must be used for that particular
    # residue pair, eg, "Epsilon-Intra-Solvent", or
    # "Epsilon-Solvent-Membrane", etc.

        self.representations = representations
        self.m = self.representations[0].prot.get_model()
        self.rs = IMP.RestraintSet(self.m, 'Cysteine_Crosslink')
        self.cbeta = cbeta
        self.epsilonmaxtrans = 0.01
        self.sigmamaxtrans = 0.1
        self.betamaxtrans = 0.01
        self.weightmaxtrans = 0.1
        self.label = "None"
        self.outputlevel = "low"
        self.betaissampled = betaissampled
        self.sigmaissampled = sigmaissampled
        self.weightissampled = weightissampled
        self.epsilonissampled = epsilonissampled

        betalower = betatuple[0]
        betaupper = betatuple[1]
        beta = 0.04
        sigma = 10.0
        betangrid = 30
        crossdataprior = 1

        # beta
        self.beta = IMP.pmi.tools.SetupNuisance(
            self.m,
            beta,
            betalower,
            betaupper,
            betaissampled).get_particle(
        )
        # sigma
        self.sigma = IMP.pmi.tools.SetupNuisance(
            self.m,
            sigma,
            sigmatuple[0],
            sigmatuple[1],
            sigmaissampled).get_particle()
        # population particle
        self.weight = IMP.pmi.tools.SetupWeight(
            self.m,
            weightissampled).get_particle(
        )

        # read the file
        fl = IMP.pmi.tools.open_file_or_inline_text(filename)
        # determine the upperlimit for epsilon
        # and also how many epsilon are needed
        self.epsilons = {}
        data = []
        for line in fl:
            t = line.split()
            if t[0][0] == "#":
                continue
            if t[5] in self.epsilons:
                if 1.0 - float(t[4]) <= self.epsilons[t[5]].get_upper():
                    self.epsilons[t[5]].set_upper(1.0 - float(t[4]))
            else:
                self.epsilons[t[5]] = IMP.pmi.tools.SetupNuisance(self.m,
                                                                  0.01, 0.01, 1.0 - float(t[4]), epsilonissampled).get_particle()
            up = self.epsilons[t[5]].get_upper()
            low = self.epsilons[t[5]].get_lower()
            if up < low:
                self.epsilons[t[5]].set_upper(low)

            data.append((int(t[0]), t[1], int(t[2]), t[3], float(t[4]), t[5]))

        # create CrossLinkData
        if not self.cbeta:
            crossdata = IMP.pmi.tools.get_cross_link_data(
                "cysteine", "cysteine_CA_FES.txt.standard",
                disttuple, omegatuple, sigmatuple, disttuple[1], disttuple[1], 1)
        else:
            crossdata = IMP.pmi.tools.get_cross_link_data(
                "cysteine", "cysteine_CB_FES.txt.standard",
                disttuple, omegatuple, sigmatuple, disttuple[1], disttuple[1], 1)

        # create grids needed by CysteineCrossLinkData
        fmod_grid = IMP.pmi.tools.get_grid(0.0, 1.0, 300, True)
        omega2_grid = IMP.pmi.tools.get_log_grid(0.001, 10000.0, 100)
        beta_grid = IMP.pmi.tools.get_log_grid(betalower, betaupper, betangrid)

        for d in data:
            print("--------------")
            print("CysteineCrossLink: attempting to create a restraint " + str(d))
            resid1 = d[0]
            chain1 = d[1]
            resid2 = d[2]
            chain2 = d[3]
            fexp = d[4]
            epslabel = d[5]

            # CysteineCrossLinkData

            ccldata = IMP.isd.CysteineCrossLinkData(
                fexp,
                fmod_grid,
                omega2_grid,
                beta_grid)

            ccl = IMP.isd.CysteineCrossLinkRestraint(
                self.m,
                self.beta,
                self.sigma,
                self.epsilons[epslabel],
                self.weight,
                crossdata,
                ccldata)

            failed = False
            for i, representation in enumerate(self.representations):

                if not self.cbeta:
                    p1 = None
                    p2 = None

                    p1 = IMP.pmi.tools.select(representation,
                                              resolution=1, name=chain1,
                                              name_is_ambiguous=False, residue=resid1)[0]

                    if p1 is None:
                        failed = True

                    p2 = IMP.pmi.tools.select(representation,
                                              resolution=1, name=chain2,
                                              name_is_ambiguous=False, residue=resid2)[0]

                    if p2 is None:
                        failed = True

                else:
                    # use cbetas
                    p1 = []
                    p2 = []
                    for t in range(-1, 2):
                        p = IMP.pmi.tools.select(representation,
                                                 resolution=1, name=chain1,
                                                 name_is_ambiguous=False, residue=resid1 + t)

                        if len(p) == 1:
                            p1 += p
                        else:
                            failed = True
                            print("\033[93m CysteineCrossLink: missing representation for residue %d of chain %s \033[0m" % (resid1 + t, chain1))

                        p = IMP.pmi.tools.select(representation,
                                                 resolution=1, name=chain2,
                                                 name_is_ambiguous=False, residue=resid2 + t)

                        if len(p) == 1:
                            p2 += p
                        else:
                            failed = True
                            print("\033[93m CysteineCrossLink: missing representation for residue %d of chain %s \033[0m" % (resid2 + t, chain2))

                if not self.cbeta:
                    if (p1 is not None and p2 is not None):
                        ccl.add_contribution(p1, p2)
                        d1 = IMP.core.XYZ(p1)
                        d2 = IMP.core.XYZ(p2)

                        print("Distance_" + str(resid1) + "_" + chain1 + ":" + str(resid2) + "_" + chain2, IMP.core.get_distance(d1, d2))

                else:
                    if (len(p1) == 3 and len(p2) == 3):
                        p11n = p1[0].get_name()
                        p12n = p1[1].get_name()
                        p13n = p1[2].get_name()
                        p21n = p2[0].get_name()
                        p22n = p2[1].get_name()
                        p23n = p2[2].get_name()

                        print("CysteineCrossLink: generating CB cysteine cross-link restraint between")
                        print("CysteineCrossLink: residue %d of chain %s and residue %d of chain %s" % (resid1, chain1, resid2, chain2))
                        print("CysteineCrossLink: between particles %s %s %s and %s %s %s" % (p11n, p12n, p13n, p21n, p22n, p23n))

                        ccl.add_contribution(p1, p2)

            if not failed:
                self.rs.add_restraint(ccl)
                ccl.set_name("CysteineCrossLink_" + str(resid1)
                             + "_" + chain1 + ":" + str(resid2) + "_" + chain2)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs)

    def get_particles_to_sample(self):
        ps = {}
        if self.epsilonissampled:
            for eps in self.epsilons.keys():
                ps["Nuisances_CysteineCrossLinkRestraint_epsilon_" + eps + "_" +
                    self.label] = ([self.epsilons[eps]], self.epsilonmaxtrans)
        if self.betaissampled:
            ps["Nuisances_CysteineCrossLinkRestraint_beta_" +
                self.label] = ([self.beta], self.betamaxtrans)
        if self.weightissampled:
            ps["Weights_CysteineCrossLinkRestraint_" +
                self.label] = ([self.weight], self.weightmaxtrans)
        if self.sigmaissampled:
            ps["Nuisances_CysteineCrossLinkRestraint_" +
                self.label] = ([self.sigma], self.sigmamaxtrans)
        return ps

    def set_output_level(self, level="low"):
                # this might be "low" or "high"
        self.outputlevel = level

    def get_restraint(self):
        return self.rs

    def get_sigma(self):
        return self.sigma

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["CysteineCrossLinkRestraint_Score_" + self.label] = str(score)
        output["CysteineCrossLinkRestraint_sigma_" +
               self.label] = str(self.sigma.get_scale())
        for eps in self.epsilons.keys():
            output["CysteineCrossLinkRestraint_epsilon_" + eps + "_" +
                   self.label] = str(self.epsilons[eps].get_scale())
        output["CysteineCrossLinkRestraint_beta_" +
               self.label] = str(self.beta.get_scale())
        for n in range(self.weight.get_number_of_states()):
            output["CysteineCrossLinkRestraint_weights_" +
                   str(n) + "_" + self.label] = str(self.weight.get_weight(n))

        if self.outputlevel == "high":
            for rst in self.rs.get_restraints():
                output["CysteineCrossLinkRestraint_Total_Frequency_" +
                       IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_name() +
                       "_" + self.label] = IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_model_frequency()
                output["CysteineCrossLinkRestraint_Standard_Error_" +
                       IMP.isd.CysteineCrossLinkRestraint.get_from(
                           rst).get_name(
                       ) + "_"
                       + self.label] = IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_standard_error()
                if len(self.representations) > 1:
                    for i in range(len(self.prots)):
                        output["CysteineCrossLinkRestraint_Frequency_Contribution_" +
                               IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_name() +
                               "_State_" + str(i) + "_" + self.label] = IMP.isd.CysteineCrossLinkRestraint.get_from(rst).get_frequencies()[i]

        return output
