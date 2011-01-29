#replica exchange class
#inspired by ISD Replica.py
#Yannick

from math import exp
from random import random,randint

class ReplicaTracker():
    
    def __init__(self,nreps,inv_temps,grid,sfo_id,
            logfile='replicanums.txt', scheme='standard', xchg='random',
            convectivelog='stirred.txt'):
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
        self.stepno = 1
        self.scheme = scheme
        self.xchg = xchg
        #scheme is one of gromacs, randomneighbors or convective
        if scheme == "convective":
            self.stirred = {}
            #which order the replicas should be chosen
            self.stirred['order'] = range(self.nreps)
            self.stirred['order'].reverse()
            #which replica is currently being stirred
            self.stirred['replica']=self.stirred['order'][0]
            #which position in the order list this replica sits in
            self.stirred['pos']=0
            #what direction should this replica should go to
            if self.stirred['replica'] != self.nreps-1:
                self.stirred['dir']=1 #up 
            else:
                self.stirred['dir']=0 #down
            #how many steps are remaining til we change stirring replica
            self.stirred['steps']=2*(self.nreps-1)
            self.convectivelog = convectivelog
        self.write_rex_stats()

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

    def gen_pairs_list_gromacs(self, dir):
        "generate ordered list of pairs based on dir"
        nreps = self.nreps
        if dir != 0 and dir != 1:
            raise ValueError, dir
        if nreps == 2:
            return [(0,1)]
        return [(2*i+dir,2*i+1+dir) for i in xrange((nreps-1)/2)]

    def gen_pairs_list_rand(self, needed = []):
        "generate list of neighboring pairs of states"
        nreps = self.nreps
        init = range(nreps)
        pairslist = []
        for (i,j) in needed:
            pairslist.append((min(i,j),max(i,j)))
            init.pop(i)
            init.pop(j)
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

    def gen_pairs_list_conv(self):
        nreps = self.nreps
        rep = self.stirred['replica']
        state = self.statenums[rep]
        if self.xchg == 'gromacs':
            dir = (state + 1 + self.stirred['dir']) % 2
            return self.gen_pairs_list_gromacs(dir)
        elif self.xchg == 'random':
            pair = (state, state + 2*self.stirred['dir'] - 1)
            return self.gen_pairs_list_rand(needed=[pair])
        else:
            raise NotImplementedError, \
                    "Unknown exchange method: %s" % self.xchg

    def gen_pairs_list(self):
        if self.scheme == 'standard':
            if self.xchg == 'gromacs':
                return self.gen_pairs_list_gromacs(self.stepno % 2)
            elif self.xchg == 'random':
                return self.gen_pairs_list_rand()
            else:
                raise NotImplementedError, \
                        "unknown exchange method: %s" % self.xchg
        elif self.scheme == 'convective':
            return self.gen_pairs_list_conv()
        else:
            raise NotImplementedError, \
                    "unknown exchange scheme: %s" % self.scheme
        
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
        steps = self.grid.gather(
                self.grid.broadcast(self.sfo_id, 'get_mc_stepsize'))
        for (i,j) in accepted:
            ri = self.replicanums[i]
            rj = self.replicanums[j]
            buf = steps[ri]
            steps[ri] = steps[rj]
            steps[rj] = buf
        self.grid.gather(
                self.grid.scatter(self.sfo_id, 'set_mc_stepsize', steps))

    def write_rex_stats(self):
        "write replica numbers as a function of state"
        fl=open(self.logfile,'a')
        fl.write('%8d ' % self.stepno)
        fl.write(' '.join(['%2d' % (i+1) for i in self.replicanums]))
        fl.write('\n')
        fl.close()
        if self.scheme == 'convective':
            fl=open(self.convectivelog, 'a')
            fl.write('%5d ' % self.stepno)
            fl.write('%2d ' % self.sticklist['replica'])
            fl.write('%2d ' % self.sticklist['dir'])
            fl.write('%2d\n' % self.sticklist['steps'])
            fl.close()


    def do_bookkeeping_before(self):
        self.stepno += 1
        if self.scheme == 'convective':
            st = self.stirred
            #check if we are done stirring this replica
            if st['steps'] == 0:
                st['pos'] = (st['pos'] + 1) % self.nreps
                st['replica'] = st['order'][pos]
                st['dir'] = 1
                st['steps'] = 2*(self.nreps - 1)
            rep = st['replica']
            state = self.statenums[rep]
            #update endpoints
            if state == self.nreps - 1:
                st['dir'] = 0
            elif state == 0:
                st['dir'] = 1
            self.stirred = st

    def do_bookkeeping_after(self, accepted):
        if self.scheme == 'convective':
            rep = self.stirred['replica']
            state = self.statenums[rep]
            dir = 2*self.stirred['dir'] - 1
            expected = (min(state,state+dir),max(state,state+dir))
            pair = (state, state + 2*self.stirred['dir'] - 1)
            if pair in accepted:
                self.stirred['steps'] -= 1

    def replica_exchange(self):
        "main entry point for replica-exchange"
        self.do_bookkeeping_before()
        energies = self.sort_per_state(self.get_energies())
        plist = self.gen_pairs_list()
        metrop = self.get_metropolis(plist,energies)
        accepted = self.try_exchanges(plist, metrop)
        self.perform_exchanges(accepted)
        self.do_bookkeeping_after(accepted)




