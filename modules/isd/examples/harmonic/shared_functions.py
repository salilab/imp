#!/usr/bin/env python
import sys,os

import IMP
import IMP.core
import IMP.atom
import IMP.container
import IMP.isd
from IMP.isd.shared_functions import sfo_common
from IMP.isd.Statistics import Statistics
from IMP.isd.Entry import Entry

from math import sqrt

kB= (1.381 * 6.02214) / 4184.0

class sfo(sfo_common):
    "shared functions object, published on all nodes"


    def init_model(self, wd, ff_temp=300.0):
        IMP.base.set_check_level(IMP.base.NONE)
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
        si = IMP.core.PairRestraint(df, (p1,p2))
        m.add_restraint(si)
        self._m = m
        self._p2 = p2
        self._p1 = p1
        print "done initting model"

    def init_simulation(self,lambda_temp=1.0, tau=500.0):
        "prepare md and mc sims"
        print "preparing sims"
        self.inv_temp = lambda_temp
        self._mc, self._nm = self.init_simulation_setup_scale_mc(self._p2,
                temperature=1/(kB*lambda_temp), floatkey=IMP.FloatKey("x"))
        print "done preparing sim"

    def init_stats(self, prefix='r01', rate=1):
        print "initting stats"
        stat = Statistics(prefix, rate)
        self.stat=stat
        self.mc_key = self.init_stats_add_mc_category(stat, coord='X')
        self.stat.add_entry('global', entry=Entry('TargetBeta', '%10f',
            self.inv_temp))
        self.stat.add_entry('global', entry=Entry('TargetTemp', '%10f',
            1/(kB*self.inv_temp)))
        self.stat.add_entry('global', entry=Entry('Potential', '%10f',
            self._m.evaluate, False))
        print "done initting stats"

    def do_mc_and_update_stepsize(self,nsteps):
        """perform mc on scales for nsteps, updating stepsizes to target
        50% acceptance. Don't make nsteps too small (say << 50).
        """
        print "running mc"
        self._mc_and_update_nm(nsteps,self._mc,self._nm, self.mc_key)
        self.stat.update(self.mc_key, 'X', self._p2.get_coordinates()[0])
        print "done running mc"

    def write_stats(self):
        #increment global counter
        self.stat.increment_counter('global', 1)
        #write statistics if necessary
        self.stat.write_stats()

    def set_inv_temp(self, inv_temp):
        "sets inverse temperature of mc and md sims (used in replica exchange)"
        self.inv_temp = inv_temp
        self._mc.set_temperature(1/self.inv_temp)

    def get_inv_temp(self):
        return self.inv_temp

    def get_state(self):
        return {'inv_temp': self.inv_temp}

    def set_state(self,state):
        self.set_inv_temp(state['inv_temp'])

if __name__ == '__main__':
    sf=sfo()
    sf.init_model('./')
    sf.init_simulation()
    sf.init_stats()
    for i in xrange(10):
        sf.do_mc_and_update_stepsize(10)
        sf.write_stats()
