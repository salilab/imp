#!/usr/bin/env python
import sys,os

import IMP
import IMP.core
import IMP.atom
import IMP.container

from math import sqrt

kB= (1.381 * 6.02214) / 4184.0

class sfo():
    "shared functions object, published on all nodes"

    def hello(self):
        return "hello world"

    def set_checklevel(self,value):
        IMP.set_check_level(value)

    def set_loglevel(self,value):
        IMP.set_log_level(value)

    def init_model(self, wd, ff_temp=300.0):
        IMP.set_check_level(IMP.NONE)
        os.chdir(wd)
        print "harmonic restraint on a unit mass particle at origin"
        m = IMP.Model()
        p1=IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p1.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        #IMP.atom.Mass.setup_particle(p1,1.0)
        p1.set_coordinates_are_optimized(False)
        p2=IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p2.set_coordinates(IMP.algebra.Vector3D(1,0,0))
        #IMP.atom.Mass.setup_particle(p2,1.0)
        p2.set_coordinates_are_optimized(True)
        p2.set_is_optimized(IMP.FloatKey("x"),True)
        p2.set_is_optimized(IMP.FloatKey("y"),False)
        p2.set_is_optimized(IMP.FloatKey("z"),False)
        ha = IMP.core.Harmonic(0,1) #gaussian with Cv = kB
        df = IMP.core.DistancePairScore(ha)
        si = IMP.core.PairRestraint(df, IMP.ParticlePair(p1,p2))
        m.add_restraint(si)
        self._m = m
        self._p2 = p2
        self._p1 = p1
        print "done initting model"

    def m(self,name,*args,**kw):
        "wrapper to call methods of m"
        func=getattr(self._m,name)
        return func(*args, **kw)

    def init_simulation(self,lambda_temp=1.0, tau=500.0, stat_rate=100):
        "prepare md and mc sims"
        print "preparing sims"
        self.inv_temp = lambda_temp
        #self.md_tau = tau
        #self._md = self._setup_md(1/(kB*lambda_temp), tau)
        self._mc, self._nm = self._setup_mc(self._p2, lambda_temp)
        self.local_counter = 0
        self.global_counter = 0
        self.stat_rate=100
        print "done preparing sim"

    def do_md(self,nsteps):
        "perform md simulation on protein for nsteps"
        self._md.optimize(nsteps)

    def do_mc_and_update_stepsize(self,nsteps):
        """perform mc on scales for nsteps, updating stepsizes to target 
        50% acceptance. Don't make nsteps too small (say << 50).
        """
        print "running mc"
        self._mc_and_update(nsteps,self._mc,self._nm)
        print "done running mc"

    def init_stats(self, prefix='r01'):
        print "initting stats"
        self.prefix=prefix
        self.statfile = prefix+'_stats.txt'
        flstat=open(self.statfile,'w')
        flstat.write("GibbsStep MCStep TargetBeta TargetTemp "
                "X Potential accept stepsize\n")
        flstat.close()
        self.naccept=0
        print "done initting stats"

    def write_stats(self,nsteps):
        flstat=open(self.statfile,'a')
        #kinetic   = self._md.get_kinetic_energy() 
        x=self._p2.get_x()
        potential = self._m.evaluate(False)
        #temp = self._md.get_kinetic_temperature(kinetic)
        acc_s,st_s = self._get_mc_stat(self._mc, self._nm,
                self.naccept,nsteps)
        self.naccept = self._mc.get_number_of_forward_steps()
        for i in [self.global_counter, self.local_counter]:
            flstat.write("%10d " % i)
        for i in [self.inv_temp, 1/(kB*self.inv_temp), 
                x, potential, acc_s, st_s]:
            flstat.write("%10f " % i)
        flstat.write('\n')
        flstat.close()

    def set_temp(self, inv_temp):
        #self._md.set_therm(2, 1.0/(kB*inv_temp), self.md_tau)
        #self._md.rescale_vel(sqrt(self.inv_temp/inv_temp))
        self.inv_temp = inv_temp
        self._mc.set_temperature(1/self.inv_temp)
    
    def get_temp(self):
        return self.inv_temp

    def get_mc_stepsize(self):
        return self._nm.get_sigma()

    def set_mc_stepsize(self, stepsize):
        self._nm.set_sigma(stepsize)

    def _setup_md(self,temp=300.0, tau=500):
        ## Molecular Dynamics (from MAX BONOMI)
        md=IMP.atom.MolecularDynamics()
        md.set_model(self._m)
        md.assign_velocities(temp)
        md.set_time_step(1.0)
        ## therm legend
        # 0 :: nve
        # 1 :: rescale velocities
        # 2 :: berendsen
        # 3 :: langevin
        #md.set_therm(0,0,0)
        #md.set_therm(1,300,0)
        md.set_therm(2,temp,tau)
        #md.set_therm(3,300,0.01)
        # metadynamics setup
        #md.mtd_setup(0.003, 10.0, -200.0, 400.0)
        return md

    def _setup_mc(self,particle,beta=1.676972322):
        "monte carlo on scale parameter"
        mc = IMP.core.MonteCarlo(self._m)
        cont=IMP.container.ListSingletonContainer(self._m)
        cont.add_particle(particle)
        nm_particle=IMP.core.NormalMover(cont,
                IMP.FloatKeys([IMP.FloatKey("x")]),1.0)
        cont=IMP.container.ListSingletonContainer(self._m)
        #why is this returning an int?
        mc.add_mover(nm_particle)
        #set same temp as MD, careful with units
        mc.set_temperature(1.0/beta)
        #allow to go uphill
        mc.set_return_best(False)
        #update particle each time
        mc.set_move_probability(1.0)
        return (mc,nm_particle)

    def _mc_and_update(self,nsteps,mc,nm):
        before = mc.get_number_of_forward_steps()
        self.global_counter += 1
        if self.stat_rate <= 0:
            self.local_counter += nsteps
            mc.optimize(nsteps)
            self.write_stats(self.stat_rate)
        else:
            for i in xrange(nsteps/self.stat_rate):
                self.local_counter += self.stat_rate
                mc.optimize(self.stat_rate)
                self.write_stats(self.stat_rate)
            remainder = nsteps % self.stat_rate
            if remainder != 0:
                mc.optimize(remainder)
                self.local_counter += remainder
                self.write_stats(remainder)
        after = mc.get_number_of_forward_steps()
        accept = float(after-before)/nsteps
        if 0.4 < accept < 0.6:
            return
        if accept < 0.05:
            accept = 0.05
        if accept > 1.0:
            accept = 1.0
        nm.set_sigma(nm.get_sigma()*2*accept)

        "helper class to gather and print statistics on a simulation"

    def _get_mc_stat(self,mc,nm,nacc,nsteps):
        "return acceptance rate and stepsize"
        stepsize = nm.get_sigma()
        n_ok=mc.get_number_of_forward_steps() - nacc
        return 100*n_ok/nsteps, stepsize

