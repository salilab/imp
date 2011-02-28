#!/usr/bin/env python
import sys,os

import IMP
import IMP.atom
import IMP.container
import IMP.isd
import IMP.saxs
from IMP.isd.shared_functions import sfo_common 

from math import sqrt

kB= (1.381 * 6.02214) / 4184.0

class sfo(sfo_common):
    "shared functions object, published on all nodes"

    def init_model(self, wd, seqfile, initpdb, noe, saxs, ff_temp=300.0):
        "loads pdb and restraints and creates particles and scales"
        # Create an IMP model and add a heavy atom-only protein from a PDB file
        self.init_model_base(wd)
        m=self._m
        #
        print "loading protein and force field"
        prot, ff, rsb, rs = self.init_model_charmm_protein_and_ff(initpdb, 
                                "data/top.lib", "data/par.lib", 
                                IMP.atom.NonWaterNonHydrogenPDBSelector(),
                                IMP.isd.RepulsiveDistancePairScore(0,1),
                                ff_temp=300.0)
        self._rs = {}
        self._rs['phys'] = rs
        self._rs['phys_bonded'] = rsb
        self._p={}
        self._p['prot'] = prot
        # 
        print "NOE restraints"
        noe_rs, prior_rs, sigma, gamma = self.init_model_NOEs(prot, 
                        seqfile, tblfile, name='NOE', prior_rs=None,
                        bounds_sigma=(1.0,0.1,100), 
                        bounds_gamma=(1.0,0.1,100))
        self._rs['NOE'] = noe_rs
        self._rs['prior'] = prior_rs
        self._p['sigma'] = sigma
        self._p['gamma'] = gamma
        #
        print "SAXS restraint"
        rs = self.init_model_standard_SAXS_restraint(prot, saxs)
        self._rs['SAXS'] = rs
        
    def init_simulation(self,lambda_temp=1.0, tau=500.0, stat_rate=1000):
        "prepare md and mc sims"
        self.inv_temp = lambda_temp
        self.md_tau = tau
        ### HMC on protein
        #restraints to use for MDMover: all but prior and saxs
        md_restraints=[self._rs['phys'],self._rs['phys_bonded'],
                self._rs['NOE']]
        #restraints to use for metropolis of HMC: all but prior
        hmc_restraints=[self._rs['phys'],self._rs['phys_bonded'],
                self._rs['NOE'], self._rs['SAXS']]
        self._hmc_mc, self._hmc_mv, self._hmc_md = \
                self.init_simulation_setup_protein_hmc(
                        self._p['prot'], temperature = 1/(kB*lambda_temp), 
                        thermostat='berendsen', coupling=tau,
                        n_md_steps = 10,
                        md_restraints=md_restraints,
                        mc_restraints=hmc_restraints)
        ### MC on sigma and gamma
        rs_scales=[self._rs['NOE'],self._rs['prior']]
        self._mc_sigma, self._nm_sigma = \
                self.init_simulation_setup_scale_mc(
                        self._p['sigma'], 1/(kB*lambda_temp),
                            mc_restraints=rs_scales, nm_stepsize=0.1)
        self._mc_sigma, self._nm_sigma = \
                self.init_simulation_setup_scale_mc(
                        self._p['gamma'], 1/(kB*lambda_temp),
                            mc_restraints=rs_scales, nm_stepsize=0.1)
        self.local_counter = 0
        self.global_counter = 0
        self.stat_rate=stat_rate

    def do_hmc(self,nsteps):
        "perform hybrid md/mc simulation on protein for nsteps"
        self.global_counter += 1
        for i in IMP.atom.get_leaves(self._p['prot']):
            IMP.core.XYZR(i).set_coordinates_are_optimized(True)
        self._p['sigma'].set_is_optimized(IMP.FloatKey("scale"),False)
        self._p['gamma'].set_is_optimized(IMP.FloatKey("scale"),False)
        self._hmc_and_update_md(nsteps, self._hmc_mc, self._hmc_mv)
        self.write_pdb(self.local_counter)

    def do_mc(self,nsteps):
        """perform mc on scales for nsteps, updating stepsizes to target 
        50% acceptance. Don't make nsteps too small (say << 50).
        """
        prot = self._p['prot']
        sigma = self._p['sigma']
        gamma = self._p['gamma']
        for i in IMP.atom.get_leaves(prot):
            IMP.core.XYZR(i).set_coordinates_are_optimized(False)
        sigma.set_is_optimized(IMP.FloatKey("scale"),True)
        gamma.set_is_optimized(IMP.FloatKey("scale"),False)
        self._mc_and_update_nm(nsteps,self._mc_sigma,self._nm_sigma)
        #sigma.set_is_optimized(IMP.FloatKey("scale"),False)
        #gamma.set_is_optimized(IMP.FloatKey("scale"),True)
        #self._mc_and_update_nm(nsteps,self._mc_gamma,self._nm_gamma)

    def write_pdb(self, counter):
        IMP.atom.write_pdb(self._p['prot'],self.prefix+"_%012d.pdb" % counter)

    def init_stats(self, prefix='p01'):
        self.prefix=prefix
        self.statfile = prefix+'_stats.txt'
        flstat=open(self.statfile,'w')
        flstat.write("1:global_step 2:local_step 3:Temp 4:Potential "
                "5:E_phys 6:E_data 7:E_prior 8:E_saxs 9:Sigma "
                "10:MC_accept_s 11:MC_stepsize_s "
                "12:Gamma 13:MC_accept_g 14:MC_stepsize_g "
                "15:HMC_accept 16:HMC_nsteps\n")
        flstat.close()
        self.naccept_s=0
        self.naccept_g=0
        self.naccept_md=0

    def write_stats(self,nsteps):
        stepno=self.local_counter
	flstat=open(self.statfile,'a')
        potential = self._m.evaluate(False)
        temp = self.inv_temp
        si=self._p['sigma'].get_scale()
        ga=self._p['gamma'].get_scale()
        acc_s = self._get_mc_acceptance(self._mc_sigma, 
                self.naccept_s, nsteps)
        st_s = self._get_mc_stepsize(self._nm_sigma)
        acc_g = self._get_mc_acceptance(self._mc_gamma, 
                self.naccept_g, nsteps)
        st_g = self._get_mc_stepsize(self._nm_gamma)
        acc_md = self._get_mc_acceptance(self._hmc_mc,
                self.naccept_md, nsteps)
        steps_md = self._hmc_mv.get_nsteps()
        self.naccept_s = self._mc_sigma.get_number_of_forward_steps()
        self.naccept_g = self._mc_gamma.get_number_of_forward_steps()
        self.naccept_md = self._hmc_mc.get_number_of_forward_steps()
        e_phys = self._rs['phys'].evaluate(False)
        e_data = self._rs['data'].evaluate(False)
        e_prior = self._rs['prior'].evaluate(False)
        e_saxs = self._rs['SAXS'].evaluate(False)
        for i in [self.global_counter, self.local_counter]:
            flstat.write("%10d " % i)
        for i in [temp, potential, 
                e_phys,e_data,e_prior, e_saxs,
                si,acc_s,st_s,ga,acc_g,st_g,acc_md,steps_md]:
            flstat.write("%10f " % i)
        flstat.write('\n')
        flstat.close()

    def set_temp(self, inv_temp):
        "sets temperature of mc and md sims (used in replica exchange)"
        self._hmc_md.set_thermostat(2, 1.0/(kB*inv_temp), self.md_tau)
        self._hmc_md.rescale_velocities(sqrt(self.inv_temp/inv_temp))
        self.inv_temp = inv_temp
        self._hmc_mc.set_temperature(1.0/self.inv_temp)
        self._mc_sigma.set_temperature(1.0/self.inv_temp)
        self._mc_gamma.set_temperature(1.0/self.inv_temp)

    def get_temp(self):
        return self.inv_temp
 
    def get_state(self):
        state={}
        state['inv_temp']=self.get_temp()
        state['gamma_mc_stepsize']=self._nm_gamma.get_sigma()
        state['sigma_mc_stepsize']=self._nm_sigma.get_sigma()
        state['hmc_stepsize']=self._hmc_mv.get_nsteps()
        return state

    def set_state(self,state):
        self.set_temp(state['inv_temp'])
        self._nm_gamma.set_sigma(state['gamma_mc_stepsize'])
        self._nm_sigma.set_sigma(state['sigma_mc_stepsize'])
        self._hmc_mv.set_nsteps(state['hmc_stepsize'])
        
    def _setup_hmc(self,temp=300.0,tau=500,n_md_steps=10):
        md = self._setup_md(temp,tau)
        #md should work on phys and data, not saxs nor prior
        md.set_restraints([self._rs['phys'],self._rs['data']])
        cont=IMP.container.ListSingletonContainer(self._m)
        cont.add_particles(IMP.atom.get_by_type(self._p['prot'], 
            IMP.atom.ATOM_TYPE))
        mdmover = IMP.atom.MDMover(cont, md, n_md_steps)
        hmc = IMP.core.MonteCarlo(self._m)
        hmc.add_mover(mdmover)
        hmc.set_temperature(kB*temp)
        #hmc.set_deactivate_restraint_set(self._rs['SAXS'])
        hmc.set_return_best(False)
        hmc.set_move_probability(1.0)
        return hmc, mdmover, md

    def _run_md_or_mc(self, nsteps, mdmc):
        "run mdmc or mc and print statistics"
        if self.stat_rate <= 0 or self.stat_rate >= nsteps:
            self.local_counter += nsteps
            mdmc.optimize(nsteps)
            self.write_stats(nsteps)
        else:
            for i in xrange(nsteps/self.stat_rate):
                self.local_counter += self.stat_rate
                fl=open('dummy','a')
                fl.write('replica %s global %d local %d\n' % (self.prefix,
                    self.global_counter, self.local_counter))
                fl.close()
                mdmc.optimize(self.stat_rate)
                self.write_stats(self.stat_rate)
            remainder = nsteps % self.stat_rate
            if remainder != 0:
                self.local_counter += remainder
                mdmc.optimize(remainder)
                self.write_stats(remainder)

    def _mc_and_update_nm(self,nsteps,mc,nm):
        "run mc, update stepsize and print statistics"
        before = mc.get_number_of_forward_steps()
        self._run_md_or_mc(nsteps, mc)
        after = mc.get_number_of_forward_steps()
        accept = float(after-before)/nsteps
        if 0.4 < accept < 0.6:
            return
        if accept < 0.05:
            accept = 0.05
        if accept > 1.0:
            accept = 1.0
        nm.set_sigma(nm.get_sigma()*2*accept)

    def _hmc_and_update_md(self,nsteps,hmc,mv):
        "run mc, update stepsize and print statistics"
        before = hmc.get_number_of_forward_steps()
        self._run_md_or_mc(nsteps, hmc)
        after = hmc.get_number_of_forward_steps()
        accept = float(after-before)/nsteps
        if 0.4 < accept < 0.6:
            return
        mdsteps = mv.get_nsteps()
        mdsteps = int(mdsteps *2**(accept-0.5))
        if mdsteps > 500:
            mdsteps = 500
        if mdsteps < 10:
            mdsteps = 10
        mv.set_nsteps(mdsteps)

    def _get_mc_acceptance(self,mc,nacc,nsteps):
        "return acceptance rate"
        n_ok=mc.get_number_of_forward_steps() - nacc
        return 100*n_ok/nsteps

    def _get_mc_stepsize(self,nm):
        "return stepsize"
        return nm.get_sigma()

if __name__ == '__main__':
    sfo=sfo()
    sfo.init_model('./','data/1edg.pdb', 'data/1edg_NOE_full_4A_complete.tbl',
            'data/iq.dat')
    sfo.init_simulation()
    sfo.init_stats()
    for i in xrange(100):
        print "\r%d" % i,
        sys.stdout.flush()
        sfo.do_hmc(10)
        sfo.do_mc(10)
