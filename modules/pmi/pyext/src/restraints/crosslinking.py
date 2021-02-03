"""@namespace IMP.pmi.restraints.crosslinking
Restraints for handling cross-linking data.
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
import IMP.pmi.io.crosslink
import IMP.pmi.restraints
from math import log
from collections import defaultdict
import itertools
import operator
import os
import ihm.location
import ihm.dataset
import warnings

class _DataRestraintSet(IMP.RestraintSet):
    """Container for restraints shown in the RMF file and in Chimera"""

    def get_static_info(self):
        # Add custom metadata to the container, for RMF
        ri = IMP.RestraintInfo()
        ri.add_string("type", "IMP.pmi.CrossLinkingMassSpectrometryRestraint")
        ri.add_float("linker_length", self.length)
        ri.add_float("slope", self.slope)
        ri.add_filename("filename", self.filename or "")
        if self.linker:
            ri.add_string("linker_auth_name", self.linker.auth_name)
            if self.linker.smiles:
                ri.add_string("linker_smiles", self.linker.smiles)
        return ri


class CrossLinkingMassSpectrometryRestraint(IMP.pmi.restraints.RestraintBase):
    """Setup cross-link distance restraints from mass spectrometry data.
    The noise in the data and the structural uncertainty of cross-linked amino-acids
    is inferred using Bayes theory of probability
    @note Wraps an IMP::isd::CrossLinkMSRestraint
    """
    _include_in_rmf = True

    def __init__(self, root_hier, database=None,
                 length=10.0,
                 resolution=None,
                 slope=0.02,
                 label=None,
                 filelabel="None",
                 attributes_for_label=None,
                 linker=None,
                 weight=1.):
        """Constructor.
        @param root_hier The canonical hierarchy containing all the states
        @param database The IMP.pmi.io.crosslink.CrossLinkDataBase
                object that contains the cross-link dataset
        @param length maximal cross-linker length (including the residue sidechains)
        @param resolution what representation resolution should the cross-link
                restraint be applied to.
        @param slope  The slope of a distance-linear scoring function that
               funnels the score when the particles are
               too far away. Suggested value 0.02.
        @param label the extra text to label the restraint so that it is
                searchable in the output
        @param filelabel automatically generated file containing missing/included/excluded
                cross-links will be labeled using this text
        @param attributes_for_label
        @param weight Weight of restraint
        @param linker description of the chemistry of the linker itself, as
               an ihm.ChemDescriptor object
               (see https://python-ihm.readthedocs.io/en/latest/main.html#ihm.ChemDescriptor).
               Common cross-linkers can be found in the `ihm.cross_linkers`
               module.
        """

        model = root_hier.get_model()

        super(CrossLinkingMassSpectrometryRestraint, self).__init__(
            model, weight=weight, label=label,
            restraint_set_class=_DataRestraintSet)

        if database is None:
            raise Exception("You must pass a database")
        if not isinstance(database, IMP.pmi.io.crosslink.CrossLinkDataBase):
            raise TypeError(
                    "CrossLinkingMassSpectrometryRestraint: database should "
                    "be an IMP.pmi.io.crosslink.CrossLinkDataBase object")
        self.database = database

        if resolution==0 or resolution is None:
            raise Exception("You must pass a resolution and it can't be zero")

        indb = open("included." + filelabel + ".xl.db", "w")
        exdb = open("excluded." + filelabel + ".xl.db", "w")
        midb = open("missing." + filelabel + ".xl.db", "w")

        self.linker = linker
        if linker is None:
            warnings.warn(
                "No linker chemistry specified; this will be guessed from the "
                "label (%s). It is recommended to specify a linker as an "
                "ihm.ChemDescriptor object (see the "
                "CrossLinkingMassSpectrometryRestraint documentation)." % label,
                IMP.pmi.ParameterWarning)
        self.rs.set_name(self.rs.get_name() + "_Data")
        self.rspsi = self._create_restraint_set("PriorPsi")
        self.rssig = self._create_restraint_set("PriorSig")
        self.rslin = self._create_restraint_set("Linear")
        # Add custom metadata (will be saved in RMF output)
        self.rs.filename = self.database.name
        self.rs.length = length
        self.rs.slope = slope
        self.rs.linker = linker

        # dummy linear restraint used for Chimera display
        self.linear = IMP.core.Linear(0, 0.0)
        self.linear.set_slope(0.0)
        dps2 = IMP.core.DistancePairScore(self.linear)

        self.psi_is_sampled = True
        self.sigma_is_sampled = True
        self.psi_dictionary={}
        self.sigma_dictionary={}
        self.xl_list=[]
        self.outputlevel = "low"

        restraints = []

        xl_groups = [p.get_cross_link_group(self)
                     for p, state in IMP.pmi.tools._all_protocol_outputs(
                                             root_hier)]

        # first add all the molecule copies as clones to the database
        copies_to_add = defaultdict(int)
        print('gathering copies')
        for xlid in self.database.xlid_iterator():
            for xl in self.database[xlid]:
                r1 = xl[self.database.residue1_key]
                c1 = xl[self.database.protein1_key]
                r2 = xl[self.database.residue2_key]
                c2 = xl[self.database.protein2_key]
                for c,r in ((c1,r1),(c2,r2)):
                    if c in copies_to_add:
                        continue
                    sel = IMP.atom.Selection(root_hier,
                                             state_index=0,
                                             molecule=c,
                                             residue_index=r,
                                             resolution=resolution).get_selected_particles()
                    if len(sel)>0:
                        copies_to_add[c] = len(sel)-1
        print(copies_to_add)
        for molname in copies_to_add:
            if copies_to_add[molname]==0:
                continue
            fo1 = IMP.pmi.io.crosslink.FilterOperator(
                    self.database.protein1_key, operator.eq, molname)
            self.database.set_value(self.database.protein1_key,
                                    molname + '.0', fo1)
            fo2 = IMP.pmi.io.crosslink.FilterOperator(
                    self.database.protein2_key, operator.eq, molname)
            self.database.set_value(self.database.protein2_key,
                                    molname + '.0', fo2)
            for ncopy in range(copies_to_add[molname]):
                self.database.clone_protein('%s.0' % molname,
                                            '%s.%i' % (molname, ncopy + 1))
        print('done pmi2 prelims')

        for xlid in self.database.xlid_iterator():
            new_contribution=True
            for xl in self.database[xlid]:

                r1 = xl[self.database.residue1_key]
                c1 = xl[self.database.protein1_key]
                r2 = xl[self.database.residue2_key]
                c2 = xl[self.database.protein2_key]

                name1 = c1
                name2 = c2
                copy1 = 0
                copy2 = 0
                if '.' in c1:
                    name1,copy1 = c1.split('.')
                if '.' in c2:
                    name2,copy2 = c2.split('.')

                # todo: check that offset is handled correctly
                ex_xls = [(p[0].add_experimental_cross_link(
                                   r1, name1, r2, name2, group), group)
                          for p, group in
                                 zip(IMP.pmi.tools._all_protocol_outputs(
                                             root_hier),
                                     xl_groups)]

                iterlist = range(len(IMP.atom.get_by_type(root_hier,
                                                          IMP.atom.STATE_TYPE)))
                for nstate, r in enumerate(iterlist):
                    # loop over every state
                    xl[self.database.state_key] = nstate
                    xl[self.database.data_set_name_key] = self.label

                    ps1 = IMP.atom.Selection(root_hier,
                                             state_index=nstate,
                                             molecule=name1,
                                             copy_index=int(copy1),
                                             residue_index=r1,
                                             resolution=resolution).get_selected_particles()
                    ps2 = IMP.atom.Selection(root_hier,
                                             state_index=nstate,
                                             molecule=name2,
                                             copy_index=int(copy2),
                                             residue_index=r2,
                                             resolution=resolution).get_selected_particles()

                    ps1 = [IMP.atom.Hierarchy(p) for p in ps1]
                    ps2 = [IMP.atom.Hierarchy(p) for p in ps2]

                    if len(ps1) > 1:
                        raise ValueError("residue %d of chain %s selects multiple particles %s" % (r1, c1, str(ps1)))
                    elif len(ps1) == 0:
                        warnings.warn(
                            "CrossLinkingMassSpectrometryRestraint: "
                            "residue %d of chain %s is not there" % (r1, c1),
                            IMP.pmi.StructureWarning)
                        midb.write(str(xl) + "\n")
                        continue

                    if len(ps2) > 1:
                        raise ValueError("residue %d of chain %s selects multiple particles %s" % (r2, c2, str(ps2)))
                    elif len(ps2) == 0:
                        warnings.warn(
                            "CrossLinkingMassSpectrometryRestraint: "
                            "residue %d of chain %s is not there" % (r2, c2),
                            IMP.pmi.StructureWarning)
                        midb.write(str(xl) + "\n")
                        continue

                    p1 = ps1[0]
                    p2 = ps2[0]

                    if p1 == p2 and r1 == r2:
                        warnings.warn(
                            "CrossLinkingMassSpectrometryRestraint: "
                            "same particle and same residue, skipping "
                            "cross-link", IMP.pmi.StructureWarning)
                        continue

                    if new_contribution:
                        print("generating a new cross-link restraint")
                        new_contribution=False
                        dr = IMP.isd.CrossLinkMSRestraint(
                            self.model,
                            length,
                            slope)
                        restraints.append(dr)


                    if self.database.sigma1_key not in xl.keys():
                        sigma1name="SIGMA"
                        xl[self.database.sigma1_key] = sigma1name
                    else:
                        sigma1name = xl[self.database.sigma1_key]
                    sigma1=self.create_sigma(sigma1name)

                    if self.database.sigma2_key not in xl.keys():
                        sigma2name="SIGMA"
                        xl[self.database.sigma2_key] = sigma2name
                    else:
                        sigma2name = xl[self.database.sigma2_key]
                    sigma2=self.create_sigma(sigma2name)

                    if self.database.psi_key not in xl.keys():
                        psiname="PSI"
                        xl[self.database.psi_key] = psiname
                    else:
                        psiname = xl[self.database.psi_key]

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
                    for p, ex_xl in zip(IMP.pmi.tools._all_protocol_outputs(
                                                root_hier),
                                        ex_xls):
                        p[0].add_cross_link(p[1], ex_xl[0], p1, p2, length,
                                            sigma1, sigma2, psi, ex_xl[1])

                    # check if the two residues belong to the same rigid body
                    if(IMP.core.RigidMember.get_is_setup(p1) and
                        IMP.core.RigidMember.get_is_setup(p2) and
                        IMP.core.RigidMember(p1).get_rigid_body() ==
                        IMP.core.RigidMember(p2).get_rigid_body()):
                        xl["IntraRigidBody"]=True
                    else:
                        xl["IntraRigidBody"]=False

                    xl_label = self.database.get_short_cross_link_string(xl)
                    xl["ShortLabel"]=xl_label
                    dr.set_name(xl_label)

                    if p1i != p2i:
                        pr = IMP.core.PairRestraint(self.model, dps2, (p1i, p2i))
                        pr.set_name(xl_label)
                        self.rslin.add_restraint(pr)

                    self.xl_list.append(xl)

                    indb.write(str(xl) + "\n")

        if len(self.xl_list) == 0:
            raise SystemError("CrossLinkingMassSpectrometryRestraint: no cross-link was constructed")
        self.xl_restraints = restraints
        lw = IMP.isd.LogWrapper(restraints,1.0)
        self.rs.add_restraint(lw)

    def __set_dataset(self, ds):
        self.database.dataset = ds
    dataset = property(lambda self: self.database.dataset, __set_dataset)

    def get_hierarchies(self):
        """ get the hierarchy """
        return self.prot

    def get_restraint_sets(self):
        """ get the restraint set """
        return self.rs

    def get_restraints(self):
        """ get the restraints in a list """
        return self.xl_restraints

    def get_restraint_for_rmf(self):
        """ get the dummy restraints to be displayed in the rmf file """
        return self.rs, self.rssig, self.rspsi

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
        sigma = IMP.pmi.tools.SetupNuisance(
            self.model, sigmainit, sigmaminnuis, sigmamaxnuis,
            self.sigma_is_sampled, name=name).get_particle()
        self.sigma_dictionary[name] = (
            sigma,
            sigmatrans,
            self.sigma_is_sampled)
        self.rssig.add_restraint(
            IMP.isd.UniformPrior(
                self.model,
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
        psi = IMP.pmi.tools.SetupNuisance(
            self.model, psiinit, psiminnuis, psimaxnuis,
            self.psi_is_sampled, name=name).get_particle()
        self.psi_dictionary[name] = (
            psi,
            psitrans,
            self.psi_is_sampled)

        self.rspsi.add_restraint(
            IMP.isd.UniformPrior(
                self.model,
                psi,
                1000000000.0,
                psimax,
                psimin))

        self.rspsi.add_restraint(IMP.isd.JeffreysRestraint(self.model, psi))
        return psi

    def get_output(self):
        """ Get the output of the restraint to be used by the IMP.pmi.output object"""
        output = super(CrossLinkingMassSpectrometryRestraint, self).get_output()

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
                    str(psiname) + self._label_suffix] = str(
                        self.psi_dictionary[psiname][0].get_scale())

        for sigmaname in self.sigma_dictionary:
            output["CrossLinkingMassSpectrometryRestraint_Sigma_" +
                   str(sigmaname) + self._label_suffix] = str(
                    self.sigma_dictionary[sigmaname][0].get_scale())


        return output

    def get_movers(self):
        """ Get all need data to construct a mover in IMP.pmi.dof class"""
        movers=[]
        if self.sigma_is_sampled:
            for sigmaname in self.sigma_dictionary:
                mover_name="Nuisances_CrossLinkingMassSpectrometryRestraint_Sigma_" + str(sigmaname) + "_" + self.label
                particle=self.sigma_dictionary[sigmaname][0]
                maxstep=(self.sigma_dictionary[sigmaname][1])
                mv=IMP.core.NormalMover([particle],
                                  IMP.FloatKeys([IMP.FloatKey("nuisance")]),maxstep)
                mv.set_name(mover_name)
                movers.append(mv)

        if self.psi_is_sampled:
            for psiname in self.psi_dictionary:
                mover_name="Nuisances_CrossLinkingMassSpectrometryRestraint_Psi_" + str(psiname) + "_" + self.label
                particle=self.psi_dictionary[psiname][0]
                maxstep=(self.psi_dictionary[psiname][1])
                mv=IMP.core.NormalMover([particle],
                                  IMP.FloatKeys([IMP.FloatKey("nuisance")]),maxstep)
                mv.set_name(mover_name)
                movers.append(mv)

        return movers


    def get_particles_to_sample(self):
        """ Get the particles to be sampled by the IMP.pmi.sampler object """
        ps = {}
        if self.sigma_is_sampled:
            for sigmaname in self.sigma_dictionary:
                ps["Nuisances_CrossLinkingMassSpectrometryRestraint_Sigma_" +
                   str(sigmaname) + self._label_suffix] =\
                    ([self.sigma_dictionary[sigmaname][0]],
                     self.sigma_dictionary[sigmaname][1])

        if self.psi_is_sampled:
            for psiname in self.psi_dictionary:
                ps["Nuisances_CrossLinkingMassSpectrometryRestraint_Psi_" +
                    str(psiname) + self._label_suffix] =\
                   ([self.psi_dictionary[psiname][0]], self.psi_dictionary[psiname][1])

        return ps


class AtomicCrossLinkMSRestraint(IMP.pmi.restraints.RestraintBase):
    """Setup cross-link distance restraints at atomic level
    The "atomic" aspect is that it models the particle uncertainty with a Gaussian
    The noise in the data and the structural uncertainty of cross-linked amino-acids
    is inferred using Bayes' theory of probability
    @note Wraps an IMP::isd::AtomicCrossLinkMSRestraint
    """

    _include_in_rmf = True

    def __init__(self,
                 root_hier,
                 xldb,
                 atom_type="NZ",
                 length=10.0,
                 slope=0.01,
                 nstates=None,
                 label=None,
                 nuisances_are_optimized=True,
                 sigma_init=5.0,
                 psi_init = 0.01,
                 one_psi=True,
                 filelabel=None,
                 weight=1.):
        """Constructor.
        Automatically creates one "sigma" per cross-linked residue and one "psis" per pair.
        Other nuisance options are available.
        @note Will return an error if the data+extra_sel don't specify two particles per XL pair.
        @param root_hier  The root hierarchy on which you'll do selection
        @param xldb       CrossLinkDataBase object
        @param atom_type  Can either be "NZ" or "CA"
        @param length     The XL linker length
        @param slope      Linear term to add to the restraint function to help when far away
        @param nstates    The number of states to model. Defaults to the number of states in root.
        @param label      The output label for the restraint
        @param nuisances_are_optimized Whether to optimize nuisances
        @param sigma_init The initial value for all the sigmas
        @param psi_init   The initial value for all the psis
        @param one_psi    Use a single psi for all restraints (if False, creates one per XL)
        @param filelabel automatically generated file containing missing/included/excluded
                cross-links will be labeled using this text
        @param weight Weight of restraint

        """

        # basic params
        self.root = root_hier
        rname = "AtomicXLRestraint"
        super(AtomicCrossLinkMSRestraint, self).__init__(
            self.root.get_model(), name="AtomicXLRestraint", label=label,
            weight=weight)
        self.xldb = xldb
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

        self.rs_psi = self._create_restraint_set("psi")
        self.rs_sig = self._create_restraint_set("sigma")
        self.rs_lin = self._create_restraint_set("linear")

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
        self.sig_low = setup_nuisance(self.m,self.rs_nuis,init_val=sigma_init,min_val=1.0,
                                      max_val=100.0,is_opt=self.nuis_opt)
        self.sig_high = setup_nuisance(self.m,self.rs_nuis,init_val=sigma_init,min_val=1.0,
                                       max_val=100.0,is_opt=self.nuis_opt)
        '''
        self._create_sigma('sigma',sigma_init)
        if one_psi:
            self._create_psi('psi',psi_init)
        else:
            for xlid in self.xldb.xlid_iterator():
                self._create_psi(xlid,psi_init)

        ### create all the XLs
        xlrs=[]
        for xlid in self.xldb.xlid_iterator():
            # create restraint for this data point
            if one_psi:
                psip = self.psi_dictionary['psi'][0].get_particle_index()
            else:
                psip = self.psi_dictionary[unique_id][0].get_particle_index()
            r = IMP.isd.AtomicCrossLinkMSRestraint(self.model,
                                                   self.length,
                                                   psip,
                                                   slope,
                                                   True)
            num_contributions=0

            # add a contribution for each XL ambiguity option within each state
            for nstate in self.nstates:
                for xl in self.xldb[xlid]:
                    r1 = xl[self.xldb.residue1_key]
                    c1 = xl[self.xldb.protein1_key].strip()
                    r2 = xl[self.xldb.residue2_key]
                    c2 = xl[self.xldb.protein2_key].strip()

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
                        warnings.warn("AtomicXLRestraint: residue %d of "
                                      "chain %s is not there" % (r1, c1),
                                      IMP.pmi.StructureWarning)
                        if filelabel is not None:
                            midb.write(str(xl) + "\n")
                        continue

                    if len(ps2) == 0:
                        warnings.warn("AtomicXLRestraint: residue %d of "
                                      "chain %s is not there" % (r2, c2),
                                      IMP.pmi.StructureWarning)
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
        rname = self.rs.get_name()
        self.rs=IMP.isd.LogWrapper(xlrs, self.weight)
        self.rs.set_name(rname)
        self.rs.set_weight(self.weight)
        self.restraint_sets = [self.rs] + self.restraint_sets[1:]

    def get_hierarchy(self):
        return self.prot

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
        sigma = IMP.pmi.tools.SetupNuisance(self.model,
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
                self.model,
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
        psi = IMP.pmi.tools.SetupNuisance(self.model,
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
                self.model,
                psi,
                1000000000.0,
                psimax,
                psimin))

        self.rs_psi.add_restraint(IMP.isd.JeffreysRestraint(self.model, psi))
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

        dummy_model=IMP.Model()
        hps = IMP.core.HarmonicDistancePairScore(self.length,1.0)
        dummy_rs=[]
        for nxl in range(self.get_number_of_restraints()):
            xl=IMP.isd.AtomicCrossLinkMSRestraint.get_from(self.rs.get_restraint(nxl))
            rs = IMP.RestraintSet(dummy_model, 'atomic_xl_'+str(nxl))
            for ncontr in range(xl.get_number_of_contributions()):
                ps=xl.get_contribution(ncontr)
                dr = IMP.core.PairRestraint(hps,[self.model.get_particle(p) for p in ps],
                                            'xl%i_contr%i'%(nxl,ncontr))
                rs.add_restraint(dr)
                dummy_rs.append(MyGetRestraint(rs))
        return dummy_rs


    def get_particles_to_sample(self):
        """ Get the particles to be sampled by the IMP.pmi.sampler object """
        ps = {}
        if self.sigma_is_sampled:
            for sigmaname in self.sigma_dictionary:
                ps["Nuisances_AtomicCrossLinkingMSRestraint_Sigma_" +
                   str(sigmaname) + self._label_suffix] = \
                    ([self.sigma_dictionary[sigmaname][0]],
                     self.sigma_dictionary[sigmaname][1])
        if self.psi_is_sampled:
            for psiname in self.psi_dictionary:
                ps["Nuisances_CrossLinkingMassSpectrometryRestraint_Psi_" +
                    str(psiname) + self._label_suffix] =\
                   ([self.psi_dictionary[psiname][0]], self.psi_dictionary[psiname][1])
        return ps

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
            IMP.isd.Scale(self.model,psip).set_scale(psi_val)
            for contr in range(xl.get_number_of_contributions()):
                sig1,sig2=xl.get_contribution_sigmas(contr)
                IMP.isd.Scale(self.model,sig1).set_scale(sig_val)

        print('loaded nuisances from file')

    def plot_violations(self,out_prefix,
                        max_prob_for_violation=0.1,
                        min_dist_for_violation=1e9,
                        coarsen=False,
                        limit_to_chains=None,
                        exclude_chains=''):
        """Create CMM files, one for each state, of all cross-links.
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
                c1=IMP.core.XYZ(self.model,pp[0]).get_coordinates()
                c2=IMP.core.XYZ(self.model,pp[1]).get_coordinates()

                r1 = IMP.atom.get_residue(IMP.atom.Atom(self.model,pp[0])).get_index()
                ch1 = IMP.atom.get_chain_id(IMP.atom.Atom(self.model,pp[0]))
                r2 = IMP.atom.get_residue(IMP.atom.Atom(self.model,pp[0])).get_index()
                ch2 = IMP.atom.get_chain_id(IMP.atom.Atom(self.model,pp[0]))

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
            idx1 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.model,idx1)),
                                      atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
            idx2 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.model,idx2)),
                                      atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
        dist = IMP.algebra.get_distance(IMP.core.XYZ(self.model,idx1).get_coordinates(),
                                        IMP.core.XYZ(self.model,idx2).get_coordinates())
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
                    idx1 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.model,pp[0])),
                                              atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
                    idx2 = IMP.atom.Selection(IMP.atom.get_residue(IMP.atom.Atom(self.model,pp[1])),
                                              atom_type=IMP.atom.AtomType("CA")).get_selected_particle_indexes()[0]
                    pp = [idx1,idx2]
                if limit_to_state is not None:
                    nstate = IMP.atom.get_state_index(IMP.atom.Atom(self.model,pp[0]))
                    if nstate!=limit_to_state:
                        continue
                    state_contrs.append(contr)
                dist = IMP.core.get_distance(IMP.core.XYZ(self.model,pp[0]),
                                             IMP.core.XYZ(self.model,pp[1]))
                if limit_to_chains is not None:
                    c1 = IMP.atom.get_chain_id(IMP.atom.Atom(self.model,pp[0]))
                    c2 = IMP.atom.get_chain_id(IMP.atom.Atom(self.model,pp[1]))
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
                pval = IMP.isd.Scale(self.model,xl.get_psi()).get_scale()
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
        output = super(AtomicCrossLinkMSRestraint, self).get_output()

        ### HACK to make it easier to see the few sigmas
        #output["AtomicXLRestraint_sigma"] = self.sigma.get_scale()
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


class CysteineCrossLinkRestraint(object):
    def __init__(self, root_hier, filename, cbeta=False,
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

        self.m = root_hier.get_model()
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
            isoptimized=False).get_particle(
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
            if not self.cbeta:
                p1 = None
                p2 = None

                p1 = IMP.atom.Selection(root_hier, resolution=1,
                                        molecule=chain1, residue_index=resid1,
                                        copy_index=0)
                p1 = p1.get_selected_particles()
                if len(p1) > 0:
                    p1 = p1[0]
                else:
                    failed = True

                p2 = IMP.atom.Selection(root_hier, resolution=1,
                                        molecule=chain2, residue_index=resid2,
                                        copy_index=0)
                p2 = p2.get_selected_particles()
                if len(p2) > 0:
                    p2 = p2[0]
                else:
                    failed = True

            else:
                # use cbetas
                p1 = []
                p2 = []
                for t in range(-1, 2):
                    p = IMP.atom.Selection(root_hier, resolution=1,
                                           molecule=chain1, copy_index=0,
                                           residue_index=resid1 + t)
                    p = p.get_selected_particles()
                    if len(p) == 1:
                        p1 += p
                    else:
                        failed = True
                        warnings.warn(
                            "CysteineCrossLink: missing representation for "
                            "residue %d of chain %s" % (resid1 + t, chain1),
                            IMP.pmi.StructureWarning)

                    p = IMP.atom.Selection(root_hier, resolution=1,
                                           molecule=chain2, copy_index=0,
                                           residue_index=resid2 + t)
                    p = p.get_selected_particles()
                    if len(p) == 1:
                        p2 += p
                    else:
                        failed = True
                        warnings.warn(
                            "CysteineCrossLink: missing representation for "
                            "residue %d of chain %s" % (resid2 + t, chain2),
                            IMP.pmi.StructureWarning)

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

        IMP.isd.Weight(
            self.weight.get_particle()
        ).set_weights_are_optimized(weightissampled)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs, add_to_rmf=True)

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
        return output


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
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rs, add_to_rmf=True)
        IMP.pmi.tools.add_restraint_to_model(self.m, self.rslin,
                add_to_rmf=True)

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
