#replica exchange class
#inspired by ISD Replica.py
#Yannick

from math import exp
from random import random,randint

class ReplicaTracker():
    
    def __init__(self,nreps,inv_temps,grid,sfo_id,logfile='replicanums.txt'):
        self.nreps = nreps
        #replica number as a function of state no
        self.replicanums = range(nreps)
        #state no as a function of replica no
        self.statenums = range(nreps)
        self.grid = grid
        self.sfo_id = sfo_id
        #expect inverse temperatures
        self.temps = inv_temps
        self.logfile = logfile
        self.stepno = 0

    def sort_per_state(self, inplist):
        "sorts a replica list per state"
        if len(inplist) != self.nreps:
            raise ValueError, 'list has wrong size'
        return [inplist[i] for i in self.replicanums]
        
    def sort_per_replica(self, inplist):
        "sorts a state list per replica number"
        if len(inplist) != self.nreps:
            raise ValueError, 'list has wrong size'
        return [inplist[i] for i in self.statenums]
        
    def get_energies(self):
        return self.grid.gather(
                self.grid.broadcast(self.sfo_id,'m','evaluate',False))

    def gen_pairs_list(self, nreps):
        "generate list of neighboring pairs of states"
        init = range(nreps)
        pairslist = []
        while len(init) > 1:
            i = randint(0,len(init)-1)
            dr = 2*randint(0,1)-1
            r=init.pop(i)
            if r+dr in init:
                init.remove(r+dr)
                pairslist.append((min(r,r+dr),max(r,r+dr)))
            elif r-dr in init:
                init.remove(r-dr)
                pairslist.append((min(r,r-dr),max(r,r-dr)))
        return sorted(pairslist)

    def get_cross_energies(self, pairslist):
        "get energies assuming all exchanges have succeeded"
        print "this is not needed for temperature replica-exchange"
        raise NotImplementedError
            

    def get_metropolis(self, pairslist, old_ene):
        """compute metropolis criterion for temperature replica exchange
        e.g. exp(Delta beta Delta E)
        """
        metrop={}
        (s1,s2) = pairslist[0]
        for (s1,s2) in pairslist:
            metrop[(s1,s2)] = \
                exp((old_ene[s2]-old_ene[s1])*(self.temps[s2]-self.temps[s1]))
        return metrop
                    
    def try_exchanges(self, plist, metrop):
        accepted = []
        for couple in plist:
            if (metrop[couple] >= 1) or (random() < metrop[couple]):
                accepted.append(couple)
        return accepted

    def perform_exchanges(self, accepted):
        "exchange given state couples both in local variables and on the grid"
        #locally
        for (i,j) in accepted:
            #states
            ri = self.replicanums[i]
            rj = self.replicanums[j]
            self.statenums[ri] = j
            self.statenums[rj] = i
            #replicas
            buf = self.replicanums[i]
            self.replicanums[i] = self.replicanums[j]
            self.replicanums[j] = buf
        #on the grid (suboptimal but who cares)
        newtemps = self.sort_per_replica(self.temps)
        self.grid.gather(self.grid.scatter(self.sfo_id, 'set_temp', newtemps))

    def write_rex_stats(self):
        "write replica numbers as a function of state"
        fl=open(self.logfile,'a')
        fl.write('%8d ' % self.stepno)
        fl.write(' '.join(['%2d' % (i+1) for i in self.replicanums]))
        fl.write('\n')
        fl.close()

    def replica_exchange(self):
        "main entry point for replica-exchange"
        self.stepno += 1
        energies = self.sort_per_state(self.get_energies())
        plist = self.gen_pairs_list(self.nreps)
        metrop = self.get_metropolis(plist,energies)
        accepted = self.try_exchanges(plist, metrop)
        self.perform_exchanges(accepted)




