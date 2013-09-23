#!/usr/bin/env python
import sys,os

import IMP
import IMP.atom
import IMP.container
import IMP.isd
import IMP.isd.shared_functions import sfo_common
import IMP.isd.Statistics import Statistics
import IMP.isd.Entry import Entry
sfo_common = IMP.isd.shared_functions.sfo_common
Statistics = IMP.isd.Statistics.Statistics
Entry = IMP.isd.Entry.Entry

import math

kB= (1.381 * 6.02214) / 4184.0

class sfo(sfo_common):
    "shared functions object, published on all nodes"

    def init_model(self, wd, seqfile, initpdb, noe, ff_temp=300.0):
        "loads pdb and restraints and creates particles and scales"
        # Create an IMP model and add a heavy atom-only protein from a PDB file
        self.init_model_base(wd)
        m=self._m
        #
        print "loading protein and force field"
        prot, ff, rsb, rs = self.init_model_charmm_protein_and_ff(initpdb,
                                "top.lib", "par.lib",
                                IMP.atom.NonWaterPDBSelector(),
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
                        seqfile, noe, name='NOE', prior_rs=None,
                        bounds_sigma=(1.0,0.1,100),
                        bounds_gamma=(1.0,0.1,100))
        self._rs['NOE'] = noe_rs
        self._rs['prior'] = prior_rs
        self._p['sigma'] = sigma
        self._p['gamma'] = gamma

    def init_simulation(self,lambda_temp=1.0, tau=500.0):
        "prepare md and mc sims"
        self.inv_temp = lambda_temp
        self.md_tau = tau
        self._md, self._os = self._setup_md(self._p['prot'],
                         thermostat = 'berendsen',
                         temperature = 1/(kB*lambda_temp),
                         coupling = tau)
        rs_scales=[self._rs['NOE'],self._rs['prior']]
        self._mc_sigma, self._nm_sigma = \
                self.init_simulation_setup_nuisance_mc(
                        self._p['sigma'], 1/(kB*lambda_temp),
                            mc_restraints=rs_scales, nm_stepsize=0.1)
        self._mc_gamma, self._nm_gamma = \
                self.init_simulation_setup_nuisance_mc(
                        self._p['gamma'], 1/(kB*lambda_temp),
                            mc_restraints=rs_scales, nm_stepsize=0.1)

    def init_stats(self, prefix='p01',rate=1):
        #print statistics in files with prefix and at a certain rate (in gibbs
        #sampling steps).
        stat = Statistics(prefix,rate)
        self.stat = stat
        #general stuff to monitor: forcefield terms
        def get_ephys():
            return self._rs['phys_bonded'].evaluate(False) \
                    + self._rs['phys'].evaluate(False)
        #the following line will raise a warning in debug mode, but it's ok.
        stat.add_entry('global', entry=Entry('E_phys', '%10f', get_ephys))
        stat.add_entry('global', entry=Entry('E_NOE', '%10f',
                                        self._rs['NOE'].evaluate, False))
        stat.add_entry('global', entry=Entry('E_prior', '%10f',
                                        self._rs['prior'].evaluate, False))
        #MD monitoring: target_temp, instant_temp, E_kinetic
        self.md_key = self.init_stats_add_md_category(stat)
        #MC monitoring: acceptance, stepsize, sigma
        self.sigma_mc_key = \
                self.init_stats_add_mc_category(stat, name='mc_sigma',
                                                        coord='sigma')
        #MC monitoring: acceptance, stepsize, gamma
        self.gamma_mc_key = \
                self.init_stats_add_mc_category(stat, name='mc_gamma',
                                                        coord='gamma')

    def do_md(self,nsteps):
        "perform md simulation on protein for nsteps"
        for i in IMP.atom.get_leaves(self._p['prot']):
            IMP.core.XYZR(i).set_coordinates_are_optimized(True)
        self._p['sigma'].set_is_optimized(IMP.FloatKey("scale"),False)
        self._p['gamma'].set_is_optimized(IMP.FloatKey("scale"),False)
        self._md.optimize(nsteps)
        self.do_md_protein_statistics(self.md_key, nsteps, self._md,
                temperature=1/(kB*self.inv_temp),
                prot_coordinates=self.get_pdb(self._p['prot']))

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
        self._mc_and_update_nm(nsteps, self._mc_sigma, self._nm_sigma,
                self.sigma_mc_key, adjust_stepsize=True)
        self.stat.update(self.sigma_mc_key, 'sigma', sigma.get_scale())
        sigma.set_is_optimized(IMP.FloatKey("scale"),False)
        gamma.set_is_optimized(IMP.FloatKey("scale"),True)
        self._mc_and_update_nm(nsteps,self._mc_gamma,self._nm_gamma,
                self.gamma_mc_key)
        self.stat.update(self.gamma_mc_key, 'gamma', gamma.get_scale())

    def write_stats(self):
        #increment global counter
        self.stat.increment_counter('global', 1)
        #write statistics if necessary
        self.stat.write_stats()

    def set_inv_temp(self, inv_temp):
        "sets inverse temperature of mc and md sims (used in replica exchange)"
        #MD: temperature and rescale velocities
        self._md.set_thermostat(2, 1.0/(kB*inv_temp), self.md_tau)
        self._md.rescale_velocities(math.sqrt(self.inv_temp/inv_temp))
        #MC: temperature
        self._mc_sigma.set_temperature(1.0/inv_temp)
        self._mc_gamma.set_temperature(1.0/inv_temp)
        #store it locally as well
        self.inv_temp = inv_temp

    def get_inv_temp(self):
        return self.inv_temp

    def get_state(self):
        """get_state and set_state work in tandem. They should only respect two
        rules:
            - get_state should return a dict that contains a key called inv_temp
              which is the inverse temperature used by replica-exchange
            - set_state should be able to set the state of this node when given
              the output of a get_state call.
        """
        state={}
        state['inv_temp']=self.get_inv_temp()
        state['gamma_mc_stepsize']=self._nm_gamma.get_sigma()
        state['sigma_mc_stepsize']=self._nm_sigma.get_sigma()
        return state

    def set_state(self,state):
        """set the state of this node according to the passed dict.
        See get_state.
        """
        self.set_inv_temp(state['inv_temp'])
        self._nm_gamma.set_sigma(state['gamma_mc_stepsize'])
        self._nm_sigma.set_sigma(state['sigma_mc_stepsize'])

if __name__ == '__main__':
    sfo=sfo()
    sfo.init_model('./','sequence.dat', 'generated2.pdb',
            'NOE_HN-full_7A_sparse100.tbl')
    sfo.init_simulation()
    sfo.init_stats()
    for i in xrange(100):
        print "\r%d" % i,
        sys.stdout.flush()
        sfo.do_mc(10)
        sfo.do_md(10)
        sfo.write_stats()
