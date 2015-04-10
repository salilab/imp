"""@namespace IMP.pmi.restraints.crosslinking_atomic
Restraints for handling crosslinking data at atomic resolution.
"""

from __future__ import print_function
import IMP
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.isd
import IMP.pmi.sampling_tools as sampling_tools
from collections import defaultdict
import os.path
import string

def setup_nuisance(m,rs,
                   init_val,
                   min_val_nuis,
                   max_val_nuis,
                   min_val_prior,
                   max_val_prior,
                   is_opt=True,
                   add_jeff=True):
    nuisance=IMP.isd.Scale.setup_particle(IMP.Particle(m),init_val)
    nuisance.set_lower(min_val_nuis)
    nuisance.set_upper(max_val_nuis)
    nuisance.set_is_optimized(nuisance.get_nuisance_key(),is_opt)
    rs.add_restraint(IMP.isd.UniformPrior(m,nuisance,1000000000.0,
                                          max_val_prior,min_val_prior))
    if add_jeff:
        rs.add_restraint(IMP.isd.JeffreysRestraint(m,nuisance.get_particle()))
    return nuisance


class RestraintSetupError(Exception):
    pass

class MyGetRestraint(object):
    def __init__(self,rs):
        self.rs=rs
    def get_restraint_for_rmf(self):
        return self.rs

class AtomicCrossLinkMSRestraint(object):
    def __init__(self,
                 root,
                 data,
                 extra_sel={'atom_type':IMP.atom.AtomType('NZ')},
                 length=10.0,
                 slope=0.01,
                 nstates=None,
                 label='',
                 max_dist=None,
                 nuisances_are_optimized=True,
                 sigma_init=5.0,
                 psi_init = 0.01,
                 one_psi=True,
                 create_nz=False):
        """Experimental ATOMIC XL restraint. Provide selections for the particles to restrain.
        Automatically creates one "sigma" per crosslinked residue and one "psis" per pair.
        Other nuisance options are available.
        \note Will return an error if the data+extra_sel don't specify two particles per XL pair.
        @param root      The root hierarchy on which you'll do selection
        @param data      CrossLinkData object
        @param extra_sel  Additional selections to add to each data point. Defaults to:
                          {'atom_type':IMP.atom.AtomType('NZ')}
        @param length     The XL linker length
        @param nstates    The number of states to model. Defaults to the number of states in root.
        @param label      The output label for the restraint
        @param nuisances_are_optimized Whether to optimize nuisances
        @param sigma_init The initial value for all the sigmas
        @param psi_init   The initial value for all the psis
        @param one_psi    Use a single psi for all restraints (if False, creates one per XL)
        @param create_nz  Coarse-graining hack - add 'NZ' atoms to every XL'd lysine
        """

        self.mdl = root.get_model()
        self.root = root
        self.weight = 1.0
        self.label = label
        self.length = length
        self.nuis_opt = nuisances_are_optimized
        self.nstates = nstates
        if nstates is None:
            self.nstates = len(IMP.atom.get_by_type(root,IMP.atom.STATE_TYPE))
        elif nstates!=len(IMP.atom.get_by_type(root,IMP.atom.STATE_TYPE)):
            print("Warning: nstates is not the same as the number of states in root")

        self.rs = IMP.RestraintSet(self.mdl, 'xlrestr')
        self.rs_nuis = IMP.RestraintSet(self.mdl, 'prior_nuis')
        self.particles=defaultdict(set)
        self.one_psi = one_psi
        self.create_nz = create_nz
        if one_psi:
            print('creating a single psi for all XLs')
        else:
            print('creating one psi for each XL')

        #### Setup two sigmas based on promiscuity of the residue ###
        psi_min_nuis = 1e-7
        psi_max_nuis = 0.4999999
        psi_min_prior = 0.01
        psi_max_prior = 0.49
        sigma_min_nuis = 1e-7
        sigma_max_nuis = 100.1
        sigma_min_prior = 1e-3
        sigma_max_prior = 100.0

        '''
        sig_threshold=4
        self.sig_low = setup_nuisance(self.mdl,self.rs_nuis,init_val=sigma_init,min_val=1.0,
                                      max_val=100.0,is_opt=self.nuis_opt)
        self.sig_high = setup_nuisance(self.mdl,self.rs_nuis,init_val=sigma_init,min_val=1.0,
                                       max_val=100.0,is_opt=self.nuis_opt)
        '''
        self.sigma = setup_nuisance(self.mdl,self.rs_nuis,
                                    init_val=sigma_init,
                                    min_val_nuis=sigma_min_nuis,
                                    max_val_nuis=sigma_max_nuis,
                                    min_val_prior=sigma_min_prior,
                                    max_val_prior=sigma_max_prior,
                                    is_opt=self.nuis_opt,
                                    add_jeff=False)
        if one_psi:
            self.psi = setup_nuisance(self.mdl,self.rs_nuis,
                                      init_val=psi_init,
                                      min_val_nuis=psi_min_nuis,
                                      max_val_nuis=psi_max_nuis,
                                      min_val_prior=psi_min_prior,
                                      max_val_prior=psi_max_prior,
                                      is_opt=self.nuis_opt,
                                      add_jeff=True)
        else:
            self.psis={}
            for unique_id in data:
                self.psis[unique_id]=setup_nuisance(self.mdl,self.rs_nuis,
                                                    init_val=psi_init,
                                                    min_val_nuis=psi_min_nuis,
                                                    max_val_nuis=psi_max_nuis,
                                                    min_val_prior=psi_min_prior,
                                                    max_val_prior=psi_max_prior,
                                                    is_opt=self.nuis_opt,
                                                    add_jeff=True)

        ### first read ahead to get the number of XL's per residue
        #num_xls_per_res=defaultdict(int)
        #for unique_id in data:
        #    for nstate in range(self.nstates):
        #        for xl in data[unique_id]:
        #            num_xls_per_res[str(xl.r1)]+=1
        #            num_xls_per_res[str(xl.r2)]+=1


        ### optionally create NZs, add to hierarchy, and create bond/angle restraints
        # consider moving this elsewhere (but it has to be done before setting up XL restraint!)
        self.bonded_pairs = []
        if self.create_nz:
            to_add=set()
            kappa = 1.0
            self.rset_bonds=IMP.RestraintSet(self.mdl,'Lysine_Sidechain_bonds')
            self.rset_angles=IMP.RestraintSet(self.mdl,'Lysine_Sidechain_angles')
            for nstate in range(self.nstates):
                for unique_id in data:
                    for xl in data[unique_id]:
                        xl_pairs = xl.get_selection(root,state_index=nstate)
                        for pp in xl_pairs:
                            to_add.add(pp[0])
                            to_add.add(pp[1])

            for ca in to_add:
                x0 = IMP.core.XYZ(ca).get_coordinates()
                res = IMP.atom.get_residue(IMP.atom.Atom(ca))
                frag = res.get_parent()
                #print('ca',ca)
                #print('res',res)
                #print('frag',frag)
                #IMP.atom.show_molecular_hierarchy(frag)

                nz = IMP.Particle(self.mdl)
                a = IMP.atom.Atom.setup_particle(nz,IMP.atom.AtomType('NZ'))
                IMP.core.XYZR.setup_particle(nz,IMP.algebra.Sphere3D(x0,1.85))
                res.add_child(a)

                # bond restraint
                h=IMP.core.HarmonicUpperBound(6.0,kappa)
                dps=IMP.core.DistancePairScore(h)
                pr=IMP.core.PairRestraint(dps,IMP.ParticlePair(ca,nz))
                self.bonded_pairs.append([ca,nz])
                self.rset_bonds.add_restraint(pr)

                # angle restraints
                hus=IMP.core.Harmonic(2.09,kappa)
                sel_pre = IMP.atom.Selection(frag,residue_index=res.get_index()-1).get_selected_particles()
                sel_post = IMP.atom.Selection(frag,residue_index=res.get_index()+1).get_selected_particles()
                if len(sel_pre)>1 or len(sel_post)>1:
                    print("SOMETHING WRONG WITH THIS FRAG")
                    print('ca',ca)
                    print('res',res)
                    print('frag',frag)
                    IMP.atom.show_molecular_hierarchy(frag)
                    exit()
                if len(sel_pre)==0:
                    nter = True
                else:
                    nter = False
                    ca_pre = sel_pre[0]
                if len(sel_post)==0:
                    cter = True
                else:
                    cter = False
                    ca_post = sel_post[0]
                #print('nter?',nter,'cter?',cter)

                if nter and not cter:
                    ar_post = IMP.core.AngleRestraint(hus,ca_post,ca,nz)
                    self.rset_angles.add_restraint(ar_post)
                elif cter and not nter:
                    ar_pre = IMP.core.AngleRestraint(hus,ca_pre,ca,nz)
                    self.rset_angles.add_restraint(ar_pre)
                elif nter and cter:
                    continue
                else:
                    hus2 = IMP.core.Harmonic(0,kappa)
                    idr = IMP.core.DihedralRestraint(hus2,ca,ca_pre,ca_post,nz)
                    self.rset_angles.add_restraint(idr)


        ### create all the XLs
        xlrs=[]
        for unique_id in data:
            # create restraint for this data point
            if one_psi:
                psip = self.psi.get_particle_index()
            else:
                psip = self.psis[unique_id].get_particle_index()

            r = IMP.isd.AtomicCrossLinkMSRestraint(self.mdl,
                                                   self.length,
                                                   psip,
                                                   slope,
                                                   True)
            xlrs.append(r)
            num_contributions=0

            # add a contribution for each XL ambiguity option within each state
            for nstate in range(self.nstates):
                for xl in data[unique_id]:
                    xl_pairs = xl.get_selection(root,state_index=nstate,
                                                **extra_sel)


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
                    sig1 = self.sigma
                    sig2 = self.sigma

                    # add each copy contribution to restraint
                    for p1,p2 in xl_pairs:
                        self.particles[nstate]|=set([p1,p2])
                        if max_dist is not None:
                            dist=IMP.core.get_distance(IMP.core.XYZ(p1),IMP.core.XYZ(p2))
                            if dist>max_dist:
                                continue
                        r.add_contribution([p1.get_index(),p2.get_index()],
                                           [sig1.get_particle_index(),sig2.get_particle_index()])
                        num_contributions+=1
                if num_contributions==0:
                    raise RestraintSetupError("No contributions!")

        print('created',len(xlrs),'XL restraints')
        self.rs=IMP.isd.LogWrapper(xlrs,self.weight)

    def set_weight(self,weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def set_label(self, label):
        self.label = label

    def add_to_model(self):
        self.mdl.add_restraint(self.rs)
        self.mdl.add_restraint(self.rs_nuis)
        if self.create_nz:
            self.mdl.add_restraint(self.rset_bonds)
            self.mdl.add_restraint(self.rset_angles)
    def get_hierarchy(self):
        return self.prot

    def get_restraint_set(self):
        return self.rs

    def create_restraints_for_rmf(self):
        """ create dummy harmonic restraints for each XL but don't add to model
        Makes it easy to see each contribution to each XL in RMF
        """
        dummy_mdl=IMP.Model()
        hps = IMP.core.HarmonicDistancePairScore(self.length,1.0)
        dummy_rs=[]
        for nxl in range(self.rs.get_number_of_restraints()):
            xl=IMP.isd.AtomicCrossLinkMSRestraint.get_from(self.rs.get_restraint(nxl))
            rs = IMP.RestraintSet(dummy_mdl, 'atomic_xl_'+str(nxl))
            for ncontr in range(xl.get_number_of_contributions()):
                ps=xl.get_contribution(ncontr)
                dr = IMP.core.PairRestraint(hps,[self.mdl.get_particle(p) for p in ps],
                                            'xl%i_contr%i'%(nxl,ncontr))
                rs.add_restraint(dr)
                dummy_rs.append(MyGetRestraint(rs))
        return dummy_rs


    def get_particles(self,state_num=None):
        """ Get particles involved in the restraint """
        if state_num is None:
            return list(reduce(lambda x,y: self.particles[x]|self.particles[y],self.particles))
        else:
            return list(self.particles[state_num])


    def get_bonded_pairs(self):
        return self.bonded_pairs

    def get_mc_sample_objects(self,max_step_sigma,max_step_psi):
        """ HACK! Make a SampleObjects class that can be used with PMI::samplers"""
        #ps=[[self.sig_low,self.sig_high,self.psi],max_step]
        psigma=[[self.sigma],max_step_sigma]
        if self.one_psi:
            ppsi=[[self.psi],max_step_psi]
        else:
            ppsi=[[self.psis[p] for p in self.psis],max_step_psi]
        ret = [sampling_tools.SampleObjects('Nuisances',psigma),
               sampling_tools.SampleObjects('Nuisances',ppsi)]
        return ret

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
        output["AtomicXLRestraint_sigma"] = self.sigma.get_scale()
        output["AtomicXLRestraint_priors"] = self.rs_nuis.unprotected_evaluate(None)
        if self.one_psi:
            output["AtomicXLRestraint_psi"] = self.psi.get_scale()
        ######

        if self.create_nz:
            output["AtomicXLRestraint_NZBonds"] = self.rset_bonds.evaluate(False)
            output["AtomicXLRestraint_NZAngles"] = self.rset_angles.evaluate(False)

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
